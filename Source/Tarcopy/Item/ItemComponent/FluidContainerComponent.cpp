#include "Item/ItemComponent/FluidContainerComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/FluidContainerData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemNetworkContext.h"

void UFluidContainerComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UFluidContainerComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	Super::GetCommands(OutCommands, Context);
}

void UFluidContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ContainedFluidId);
	DOREPLIFETIME(ThisClass, Amount);
}

void UFluidContainerComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UFluidContainerComponent::OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext)
{
	Super::OnExecuteAction(InInstigator, NetworkContext);

	if (NetworkContext.ActionTag == TEXT("Fill"))
	{
		Fill(NetworkContext.FloatParams[0]);
	}
}

void UFluidContainerComponent::Fill(float InAmount)
{
}

void UFluidContainerComponent::OnRep_PrintFluid()
{
	//UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s: %d left"), Amount));
	if (OnUpdatedItemComponent.IsBound())
	{
		OnUpdatedItemComponent.Broadcast();
	}
}

void UFluidContainerComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::FluidContainerTable)->FindRow<FFluidContainerData>(ItemData->ItemId, FString(""));
}

const FFluidContainerData* UFluidContainerComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
