#include "Item/ItemComponent/ClothingComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/ClothData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemCommand/EquipCommand.h"
#include "Character/MyCharacter.h"
#include "Item/EquipComponent.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/InventoryData.h"

void UClothingComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UClothingComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));
	FText TextItemName = OwnerItemData->TextName;
	ensureMsgf(GetData() != nullptr, TEXT("No ClothData"));

	if (Context.Instigator.IsValid() == false)
		return;

	UEquipComponent* EquipComponent = Context.Instigator->FindComponentByClass<UEquipComponent>();
	UPlayerInventoryComponent* InventoryComponent = Context.Instigator->FindComponentByClass<UPlayerInventoryComponent>();
	if (IsValid(EquipComponent) == false || IsValid(InventoryComponent) == false)
		return;

	bool bIsEquipped = IsValid(GetOwnerCharacter()) == true;
	UEquipCommand* EquipCommand = NewObject<UEquipCommand>(this);
	EquipCommand->TargetItem = GetOwnerItem();
	EquipCommand->TextDisplay = FText::Format(
		FText::FromString(bIsEquipped == true ? TEXT("Unequip {0}") : TEXT("Equip {0}")),
		TextItemName);
	EquipCommand->bEquip = !bIsEquipped;
	EquipCommand->BodyLocation = Data->BodyLocation;
	// 인벤토리에서 공간 있는지 체크해야 함
	UInventoryData* InventoryData = InventoryComponent->GetPlayerInventoryData();
	FIntPoint Origin;
	bool bRotated;
	if (bIsEquipped == true)
	{
		EquipCommand->bExecutable = IsValid(InventoryData) == true && InventoryData->CanAddItem(OwnerItem.Get(), Origin, bRotated) == true;
	}
	else
	{
		TArray<UItemInstance*> ReplaceItems;
		EquipComponent->GetNeedToReplace(Data->BodyLocation, ReplaceItems);
		if (ReplaceItems.Num() == 1)
		{
			EquipCommand->bExecutable = IsValid(InventoryData) == true && InventoryData->CanAddItem(OwnerItem.Get(), Origin, bRotated, ReplaceItems[0]) == true;
		}
		else
		{
			EquipCommand->bExecutable = true;
		}
	}
	OutCommands.Add(EquipCommand);
}

void UClothingComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UClothingComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::ClothTable)->FindRow<FClothData>(ItemData->ItemId, FString(""));
}

const FClothData* UClothingComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
