#include "Item/ItemNetworkComponent.h"
#include "Item/ItemNetworkContext.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "Item/CraftSubsystem.h"

UItemNetworkComponent::UItemNetworkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UItemNetworkComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UItemNetworkComponent::ServerRPC_ExecuteItemAction_Implementation(const FItemNetworkContext& NetworkContext)
{
	if (IsValid(NetworkContext.TargetItemComponent) == false)
		return;

	NetworkContext.TargetItemComponent->ExecuteAction(GetOwner(), NetworkContext);
}

void UItemNetworkComponent::ServerRPC_ExecuteCraft_Implementation(const FName& CraftId)
{
	UCraftSubsystem* CraftSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCraftSubsystem>();
	if (IsValid(CraftSubsystem) == false)
		return;

	CraftSubsystem->ExecuteCraft(GetOwner(), CraftId);
}
