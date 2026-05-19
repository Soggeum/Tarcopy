#include "Item/ItemCommand/OpenContainerCommand.h"
#include "Item/ItemComponent/ContainerComponent.h"
#include "UI/UISubsystem.h"

void UOpenContainerCommand::OnExecute(const FItemCommandContext& Context)
{
	if (OwnerComponent.IsValid() == false)
		return;

	UUISubsystem* UISubSystem = GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubSystem) == false)
		return;

	UISubSystem->ShowInventoryUI(OwnerComponent->GetInventoryData());
}
