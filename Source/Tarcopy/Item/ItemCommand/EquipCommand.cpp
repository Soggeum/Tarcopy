#include "Item/ItemCommand/EquipCommand.h"
#include "Item/ItemInstance.h"
#include "Item/EquipComponent.h"
#include "Character/MyCharacter.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"

void UEquipCommand::OnExecute(const FItemCommandContext& Context)
{
	if (TargetItem.IsValid() == false)
		return;

	if (Context.Instigator.IsValid() == false)
		return;

	UEquipComponent* EquipComponent = Context.Instigator->FindComponentByClass<UEquipComponent>();
	if (IsValid(EquipComponent) == false)
		return;

	if (bEquip == true)
	{
		EquipComponent->ServerRPC_EquipItem(BodyLocation, TargetItem.Get());
	}
	else
	{
		EquipComponent->ServerRPC_UnequipItem(TargetItem.Get());
	}
}
