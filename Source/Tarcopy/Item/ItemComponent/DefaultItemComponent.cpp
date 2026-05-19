#include "Item/ItemComponent/DefaultItemComponent.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemInstance.h"
#include "Item/ItemCommand/DropCommand.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"

void UDefaultItemComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UDefaultItemComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));
	FText TextItemName = OwnerItemData->TextName;

	AItemWrapperActor* WrapperActor = this->GetTypedOuter<AItemWrapperActor>();
	UDropCommand* DropCommand = NewObject<UDropCommand>(this);
	DropCommand->OwnerItem = GetOwnerItem();
	DropCommand->TextDisplay = FText::Format(FText::FromString(TEXT("Drop {0}")), TextItemName);
	DropCommand->bExecutable = IsValid(WrapperActor) == false;
	OutCommands.Add(DropCommand);
}
