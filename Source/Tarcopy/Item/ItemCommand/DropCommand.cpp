#include "Item/ItemCommand/DropCommand.h"
#include "Item/ItemInstance.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/Data/ItemData.h"
#include "Item/EquipComponent.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/InventoryData.h"

void UDropCommand::OnExecute(const FItemCommandContext& Context)
{
	if (OwnerItem.IsValid() == false)
		return;

	if (Context.Instigator.IsValid() == false)
		return;

	UInventoryData* Inventory = OwnerItem->GetOwnerInventory();
	if (IsValid(Inventory) == true)
	{
		UPlayerInventoryComponent* InventoryComponent = Context.Instigator->FindComponentByClass<UPlayerInventoryComponent>();
		if (IsValid(InventoryComponent) == true)
		{
			InventoryComponent->RequestDropItemToWorld(Inventory, OwnerItem.Get(), false);
		}
	}
	else
	{
		UEquipComponent* EquipComponent = Context.Instigator->FindComponentByClass<UEquipComponent>();
		if (IsValid(EquipComponent) == true)
		{
			EquipComponent->ServerRPC_UnequipItem(OwnerItem.Get(), EUnequipType::Drop);
		}
	}
}
