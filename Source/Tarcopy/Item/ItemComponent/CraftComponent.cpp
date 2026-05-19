#include "Item/ItemComponent/CraftComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/CraftSubsystem.h"
#include "Item/Data/ItemData.h"
#include "Item/Data/CraftData.h"
#include "Item/ItemCommand/CraftCommand.h"
#include "Character/MyCharacter.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemInstance.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/ItemSpawnSubsystem.h"

void UCraftComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UCraftComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	UCraftSubsystem* CraftSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCraftSubsystem>();
	if (IsValid(CraftSubsystem) == false)
		return;

	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));

	FName ItemId = OwnerItemData->ItemId;
	const FCraftRecipe* Recipe = CraftSubsystem->GetCraftRecipe(ItemId);
	if (Recipe == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	const UDataTable* ItemTable = IsValid(DataTableSubsystem) == true ? DataTableSubsystem->GetTable(EDataTableType::ItemTable) : nullptr;
	if (IsValid(ItemTable) == false)
		return;

	TArray<UInventoryData*> InventoryDatas;
	if (Context.Instigator.IsValid() == false)
		return;

	AMyCharacter* Character = Cast<AMyCharacter>(Context.Instigator.Get());
	if (IsValid(Character) == false)
		return;

	Character->GetNearbyInventoryDatas(InventoryDatas);
	
	for (const auto& CraftDataHandle : Recipe->CraftDataHandles)
	{
		FCraftData* CraftData = CraftDataHandle.DataTable->FindRow<FCraftData>(CraftDataHandle.RowName, FString(""));
		if (CraftData != nullptr)
		{
			TArray<FString> GainedItemStrings;
			for (const auto& GainedItem : CraftData->GainedItems)
			{
				const FItemData* GainedItemData = ItemTable->FindRow<FItemData>(GainedItem.Key, FString(""));
				if (GainedItemData != nullptr)
				{
					FString GainedItemString = 
						FString::FromInt(GainedItem.Value) +
						TEXT(" ") +
						GainedItemData->TextName.ToString();
					GainedItemStrings.Add(GainedItemString);
				}
			}

			FString JoinedString = FString::Join(GainedItemStrings, TEXT(", "));

			UCraftCommand* CraftCommand = NewObject<UCraftCommand>(this);
			CraftCommand->TextDisplay = FText::Format(FText::FromString(TEXT("Craft {0}")), FText::FromString(JoinedString));
			CraftCommand->bExecutable = true;			
			for (const auto& Ingredient : CraftData->IngredientItems)
			{
				TArray<UItemInstance*> OutCandidates;
				for (const auto& InventoryData : InventoryDatas)
				{
					if (IsValid(InventoryData) == false)
						continue;

					InventoryData->GetItemCountByItemId(Ingredient.Key, OutCandidates);
				}

				if (OutCandidates.Num() < Ingredient.Value)
				{
					CraftCommand->bExecutable = false;
					break;
				}
			}
			CraftCommand->CraftTargetId = CraftDataHandle.RowName;
			OutCommands.Add(CraftCommand);
		}
	}
}

void UCraftComponent::OnExecuteAction(AActor* InInstigator, const FItemNetworkContext& ActionContext)
{
	Super::OnExecuteAction(InInstigator, ActionContext);

	//ExecuteCraft();
}
