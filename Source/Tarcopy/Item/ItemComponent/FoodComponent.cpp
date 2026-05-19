#include "Item/ItemComponent/FoodComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/FoodData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemCommand/ItemNetworkCommand.h"
#include "Net/UnrealNetwork.h"
#include "Character/MyCharacter.h"
#include "Character/MoodleComponent.h"
#include "Item/ItemNetworkContext.h"

const float UFoodComponent::TotalAmount = 4;

void UFoodComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UFoodComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));
	FText TextItemName = OwnerItemData->TextName;

	ensureMsgf(GetData() != nullptr, TEXT("No FoodData"));

	UItemNetworkCommand* IngestQuarterCommand = NewObject<UItemNetworkCommand>(this);
	FItemNetworkContext IngestQuarterActionContext;
	IngestQuarterActionContext.TargetItemComponent = this;
	IngestQuarterActionContext.ActionTag = TEXT("Ingest");
	IngestQuarterActionContext.FloatParams.Add(1.0f);
	IngestQuarterCommand->ActionContext = IngestQuarterActionContext;
	IngestQuarterCommand->TextDisplay = FText::FromString(TEXT("Ingest 1/4"));
	IngestQuarterCommand->bExecutable = Amount >= 1;
	OutCommands.Add(IngestQuarterCommand);

	UItemNetworkCommand* IngestHalfCommand = NewObject<UItemNetworkCommand>(this);
	FItemNetworkContext IngestHalfActionContext;
	IngestHalfActionContext.TargetItemComponent = this;
	IngestHalfActionContext.ActionTag = TEXT("Ingest");
	IngestHalfActionContext.FloatParams.Add(2.0f);
	IngestHalfCommand->ActionContext = IngestHalfActionContext;
	IngestHalfCommand->TextDisplay = FText::FromString(TEXT("Ingest 1/2"));
	IngestHalfCommand->bExecutable = Amount >= 2;
	OutCommands.Add(IngestHalfCommand);
	
	UItemNetworkCommand* IngestAllCommand = NewObject<UItemNetworkCommand>(this);
	FItemNetworkContext IngestAllActionContext;
	IngestAllActionContext.TargetItemComponent = this;
	IngestAllActionContext.ActionTag = TEXT("Ingest");
	IngestAllActionContext.FloatParams.Add(Amount);
	IngestAllCommand->ActionContext = IngestAllActionContext;
	IngestAllCommand->TextDisplay = FText::FromString(TEXT("Ingest All"));
	IngestAllCommand->bExecutable = Amount >= 1;
	OutCommands.Add(IngestAllCommand);
}

void UFoodComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Amount);
}

void UFoodComponent::OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext)
{
	Super::OnExecuteAction(InInstigator, NetworkContext);

	if (NetworkContext.ActionTag == TEXT("Ingest"))
	{
		Ingest(InInstigator, NetworkContext.FloatParams[0]);
	}
}

void UFoodComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UFoodComponent::Ingest(AActor* InInstigator, int32 ConsumeAmount)
{
	if (GetData() == nullptr)
		return;

	if (ConsumeAmount > Amount)
		return;

	UMoodleComponent* Moodle = IsValid(InInstigator) == true ? InInstigator->FindComponentByClass<UMoodleComponent>() : nullptr;
	if (IsValid(Moodle) == false)
		return;

	float RestoreRatio = (float)ConsumeAmount / TotalAmount;
	Moodle->RestoreHunger(Data->Hunger * RestoreRatio);
	Moodle->RestoreThirst(Data->Thirst * RestoreRatio);

	Amount -= ConsumeAmount;
	OnRep_PrintAmount();

	if (Amount <= 0 && OwnerItem.IsValid() == true)
	{
		OwnerItem->RemoveFromSource();
	}
}

void UFoodComponent::OnRep_PrintAmount()
{
	if (OnUpdatedItemComponent.IsBound())
	{
		OnUpdatedItemComponent.Broadcast();
	}
}

void UFoodComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::FoodTable)->FindRow<FFoodData>(ItemData->ItemId, FString(""));
}

const FFoodData* UFoodComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
