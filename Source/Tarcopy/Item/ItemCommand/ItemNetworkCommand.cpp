#include "Item/ItemCommand/ItemNetworkCommand.h"
#include "Item/ItemNetworkComponent.h"

void UItemNetworkCommand::OnExecute(const FItemCommandContext& Context)
{
	if (Context.Instigator.IsValid() == false)
		return;

	UItemNetworkComponent* ItemInteractComponent = Context.Instigator->FindComponentByClass<UItemNetworkComponent>();
	if (IsValid(ItemInteractComponent) == false)
		return;

	ItemInteractComponent->ServerRPC_ExecuteItemAction(ActionContext);
}
