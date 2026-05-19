#include "Item/ItemComponent/MedicalComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/MedicalData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemCommand/ItemNetworkCommand.h"
#include "Net/UnrealNetwork.h"
#include "Item/ItemNetworkContext.h"
#include "Common/HealthComponent.h"

void UMedicalComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UMedicalComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const FItemCommandContext& Context)
{
	Super::GetCommands(OutCommands, Context);

	if (Context.Instigator.IsValid() == false)
		return;

	UHealthComponent* HealthComponent = Context.Instigator->FindComponentByClass<UHealthComponent>();
	if (IsValid(HealthComponent) == false)
		return;

	ensureMsgf(GetData() != nullptr, TEXT("No MedicalData"));

	UItemNetworkCommand* RepairCommand = NewObject<UItemNetworkCommand>(this);
	FItemNetworkContext RepairContext;
	RepairContext.TargetItemComponent = this;
	RepairContext.ActionTag = TEXT("RestoreHealth");
	RepairCommand->ActionContext = RepairContext;
	RepairCommand->TextDisplay = FText::FromString(FString::Printf(TEXT("Restore HP %.1f"), Data->RestoreAmount));
	RepairCommand->bExecutable = !FMath::IsNearlyEqual(HealthComponent->GetMaxHP(), HealthComponent->GetCurrentHP());
	OutCommands.Add(RepairCommand);
}

void UMedicalComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UMedicalComponent::OnExecuteAction(AActor* InInstigator, const FItemNetworkContext& NetworkContext)
{
	Super::OnExecuteAction(InInstigator, NetworkContext);

	if (NetworkContext.ActionTag == TEXT("RestoreHealth"))
	{
		RestoreHealth(InInstigator);
	}
}

void UMedicalComponent::RestoreHealth(AActor* InInstigator)
{
	if (OwnerItem.IsValid() == false)
		return;

	if (GetData() == nullptr)
		return;

	if (IsValid(InInstigator) == false)
		return;

	UHealthComponent* HealthComponent = InInstigator->FindComponentByClass<UHealthComponent>();
	if (IsValid(HealthComponent) == false)
		return;

	HealthComponent->RestoreHP(Data->RestoreAmount);
	OwnerItem->RemoveFromSource();
}

void UMedicalComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::MedicalTable)->FindRow<FMedicalData>(ItemData->ItemId, FString(""));
}

const FMedicalData* UMedicalComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
