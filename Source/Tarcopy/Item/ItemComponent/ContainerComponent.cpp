#include "Item/ItemComponent/ContainerComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/ContainerData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemCommand/OpenContainerCommand.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"

void UContainerComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);

	if (GetData() == nullptr)
		return;

	InventoryData = NewObject<UInventoryData>(this);
	InventoryData->Init(Data->ContainerBound);

	TArray<FString> ItemNames = { TEXT("Rag0"), TEXT("Rag0"), TEXT("Leather0"), TEXT("Leather0"), TEXT("WoodStick0"), TEXT("SteelBar0"), TEXT("Food0") };

	for (const auto& ItemName : ItemNames)
	{
		UItemInstance* NewItem = NewObject<UItemInstance>(this);
		NewItem->SetItemId(FName(ItemName));
		FIntPoint Origin;
		bool bRotated;
		if (InventoryData->CanAddItem(NewItem, Origin, bRotated) == true)
		{
			InventoryData->TryAddItem(NewItem, Origin, bRotated);
		}
	}
}

void UContainerComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));
	FText TextItemName = OwnerItemData->TextName;

	ensureMsgf(GetData() != nullptr, TEXT("No ContainerData"));

	UOpenContainerCommand* OpenContainerCommand = NewObject<UOpenContainerCommand>(this);
	OpenContainerCommand->OwnerComponent = this;
	OpenContainerCommand->TextDisplay = FText::Format(FText::FromString(TEXT("Open Inventory of {0}")), TextItemName);
	OpenContainerCommand->bExecutable = true;
	OutCommands.Add(OpenContainerCommand);
}

bool UContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (IsValid(InventoryData) == true)
	{
		bWroteSomething |= InventoryData->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	return bWroteSomething;
}

void UContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryData);
}

void UContainerComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UContainerComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::ContainerTable)->FindRow<FContainerData>(ItemData->ItemId, FString(""));
}

const FContainerData* UContainerComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
