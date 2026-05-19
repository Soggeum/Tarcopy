#include "Item/ItemComponent/DurabilityComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/DurabilityData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemCommand/ItemNetworkCommand.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemNetworkContext.h"

void UDurabilityComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);

	if (GetData() == nullptr)
		return;

	Condition = Data->MaxCondition;
	OnRep_PrintCondition();
}

void UDurabilityComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));

	ensureMsgf(GetData() != nullptr, TEXT("No DurabilityData"));

	UItemNetworkCommand* RepairCommand = NewObject<UItemNetworkCommand>(this);
	FItemNetworkContext IngestAllActionContext;
	IngestAllActionContext.TargetItemComponent = this;
	IngestAllActionContext.ActionTag = TEXT("RestoreDurability");
	IngestAllActionContext.FloatParams.Add(1.0f);
	RepairCommand->ActionContext = IngestAllActionContext;
	RepairCommand->TextDisplay = FText::FromString(FString::Printf(TEXT("Repair %.1f"), 1.0f));
	RepairCommand->bExecutable = true;
	OutCommands.Add(RepairCommand);
}

void UDurabilityComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Condition);
}

void UDurabilityComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UDurabilityComponent::OnExecuteAction(AActor* InInstigator, const FItemNetworkContext& NetworkContext)
{
	Super::OnExecuteAction(InInstigator, NetworkContext);

	if (NetworkContext.ActionTag == TEXT("RestoreDurability"))
	{
		RestoreDurability(NetworkContext.FloatParams[0]);
	}
}

void UDurabilityComponent::LoseDurability(float Amount)
{
	Condition -= Amount;
	Condition = FMath::Max(Condition, 0.0f);

	if (Condition <= 0.0f)
	{
		if (OwnerItem.IsValid() == true)
		{
			OwnerItem->RemoveFromSource();
		}
	}

	OnRep_PrintCondition();
}

void UDurabilityComponent::RestoreDurability(float Amount)
{
	if (GetData() == nullptr)
		return;

	Condition += Amount;
	Condition = FMath::Min(Condition, Data->MaxCondition);

	OnRep_PrintCondition();
}

void UDurabilityComponent::OnRep_PrintCondition()
{
	if (OnUpdatedItemComponent.IsBound())
	{
		OnUpdatedItemComponent.Broadcast();
	}
}

void UDurabilityComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::DurabilityTable)->FindRow<FDurabilityData>(ItemData->ItemId, FString(""));
}

const FDurabilityData* UDurabilityComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
