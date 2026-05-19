#include "Item/CraftSubsystem.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/CraftData.h"
#include "Item/ItemSpawnSubsystem.h"
#include "Character/MyCharacter.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemInstance.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"

void UCraftSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCraftSubsystem::LoadRecipes()
{
	UDataTableSubsystem* DataTableSubsystem = IsValid(GetGameInstance()) == true ? GetGameInstance()->GetSubsystem<UDataTableSubsystem>() : nullptr;
	if (IsValid(DataTableSubsystem) == true)
	{
		const UDataTable* CraftTable = DataTableSubsystem->GetTable(EDataTableType::CraftTable);
		if (CraftTable == nullptr)
			return;

		auto& CraftRows = CraftTable->GetRowMap();
		for (const auto& Pair : CraftRows)
		{
			const FCraftData* CraftData = (FCraftData*)Pair.Value;
			if (CraftData == nullptr)
				continue;

			for (const auto& [IngredientId, IngredientCount] : CraftData->IngredientItems)
			{
				if (CraftRecipes.Contains(IngredientId) == false)
				{
					CraftRecipes.Add(IngredientId, FCraftRecipe());
				}

				FDataTableRowHandle NewRowHandle;
				NewRowHandle.DataTable = CraftTable;
				NewRowHandle.RowName = Pair.Key;
				CraftRecipes[IngredientId].CraftDataHandles.Add(NewRowHandle);
			}
		}
	}

	bIsLoaded = true;
}

const FCraftRecipe* UCraftSubsystem::GetCraftRecipe(const FName& IngredientId)
{
	if (bIsLoaded == false)
	{
		LoadRecipes();
	}
	return CraftRecipes.Find(IngredientId);
}

void UCraftSubsystem::ExecuteCraft(AActor* InInstiagor, const FName& CraftId)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
		return;

	UDataTableSubsystem* DataTableSubsystem = World->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	const UDataTable* CraftTable = IsValid(DataTableSubsystem) == true ? DataTableSubsystem->GetTable(EDataTableType::CraftTable) : nullptr;
	if (IsValid(CraftTable) == false)
		return;

	const FCraftData* CraftData = CraftTable->FindRow<FCraftData>(CraftId, FString(""));
	if (CraftData == nullptr)
		return;

	AMyCharacter* InstigatorCharacter = Cast<AMyCharacter>(InInstiagor);
	if (IsValid(InstigatorCharacter) == false)
		return;

	TArray<UInventoryData*> InventoryDatas;
	InstigatorCharacter->GetNearbyInventoryDatas(InventoryDatas);

	bool bCanCraft = true;
	TMap<FName, TArray<UItemInstance*>> ItemSourcesMap;
	for (const auto& Ingredient : CraftData->IngredientItems)
	{
		TArray<UItemInstance*>& ItemSources = ItemSourcesMap.FindOrAdd(Ingredient.Key);
		for (const auto& InventoryData : InventoryDatas)
		{
			if (IsValid(InventoryData) == false)
				continue;

			TArray<UItemInstance*> OutCandidates;
			InventoryData->GetItemCountByItemId(Ingredient.Key, ItemSources);

			if (ItemSources.Num() >= Ingredient.Value)
				break;
		}

		if (ItemSources.Num() < Ingredient.Value)
		{
			bCanCraft = false;
			break;
		}
	}

	if (bCanCraft == false)
		return;

	for (const auto& Pair : ItemSourcesMap)
	{
		int32 NeedToRemove = CraftData->IngredientItems[Pair.Key];
		const TArray<UItemInstance*>& ItemSources = Pair.Value;
		NeedToRemove = FMath::Min(NeedToRemove, ItemSources.Num());
		for (int32 Idx = 0; Idx < NeedToRemove; ++Idx)
		{
			if (IsValid(ItemSources[Idx]) == false)
				continue;

			ItemSources[Idx]->RemoveFromSource();
		}
	}

	UItemSpawnSubsystem* ItemSpawnSubsystem = GetWorld()->GetSubsystem<UItemSpawnSubsystem>();
	if (IsValid(ItemSpawnSubsystem) == false)
		return;

	for (const auto& GainedItem : CraftData->GainedItems)
	{
		for (int32 Count = 0; Count < GainedItem.Value; ++Count)
		{
			ItemSpawnSubsystem->SpawnItemAtGround(InstigatorCharacter, GainedItem.Key);
		}
	}
}