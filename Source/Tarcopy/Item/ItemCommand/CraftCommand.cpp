#include "Item/ItemCommand/CraftCommand.h"
#include "Item/ItemComponent/CraftComponent.h"
#include "Item/ItemNetworkComponent.h"

void UCraftCommand::OnExecute(const FItemCommandContext& Context)
{
	if (Context.Instigator.IsValid() == false)
		return;

	UItemNetworkComponent* ItemNetworkComponent = Context.Instigator->FindComponentByClass<UItemNetworkComponent>();
	if (IsValid(ItemNetworkComponent) == false)
		return;

	ItemNetworkComponent->ServerRPC_ExecuteCraft(CraftTargetId);
}
