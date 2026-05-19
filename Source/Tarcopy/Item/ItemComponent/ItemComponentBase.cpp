#include "Item/ItemComponent/ItemComponentBase.h"
#include "Item/ItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

bool UItemComponentBase::IsSupportedForNetworking() const
{
	return true;
}

void UItemComponentBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, OwnerItem);
}

int32 UItemComponentBase::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	return IsValid(Owner) == true ? Owner->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local;
}

bool UItemComponentBase::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	if (UNetDriver* NetDriver = IsValid(Owner) == true ? Owner->GetNetDriver() : nullptr)
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

bool UItemComponentBase::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Channel->ReplicateSubobject(this, *Bunch, *RepFlags);
	if (OwnerItem.IsValid() == true)
	{
		bWroteSomething |= Channel->ReplicateSubobject(OwnerItem.Get(), *Bunch, *RepFlags);
	}
	return bWroteSomething;
}

void UItemComponentBase::ExecuteAction(AActor* InInstigator, const FItemNetworkContext& NetworkContext)
{
	if (IsValid(InInstigator) == false)
		return;

	OnExecuteAction(InInstigator, NetworkContext);
}

void UItemComponentBase::SetOwnerItem(UItemInstance* InOwnerItem)
{
	OwnerItem = InOwnerItem;

	OnRep_SetComponent();
}

UItemInstance* UItemComponentBase::GetOwnerItem() const
{
	return OwnerItem.IsValid() == true ? OwnerItem.Get() : nullptr;
}

ACharacter* UItemComponentBase::GetOwnerCharacter() const
{
	return OwnerItem.IsValid() == true ? OwnerItem->GetOwnerCharacter() : nullptr;
}

UInventoryData* UItemComponentBase::GetOwnerInventory() const
{
	return OwnerItem.IsValid() == true ? OwnerItem->GetOwnerInventory() : nullptr;
}

bool UItemComponentBase::HasAuthority() const
{
	return OwnerItem.IsValid() == true ? OwnerItem->HasAuthority() : false;
}

const FItemData* UItemComponentBase::GetOwnerItemData() const
{
	return OwnerItem.IsValid() == true ? OwnerItem->GetData() : nullptr;
}
