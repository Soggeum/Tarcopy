#include "Item/Temp/UW_TempItem.h"
#include "Item/Temp/UW_TempInteract.h"
#include "Components/TextBlock.h"
#include "Item/Data/ItemData.h"
#include "Components/PanelWidget.h"
#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "Item/ItemComponent/MeleeWeaponComponent.h"
#include "Item/ItemCommand/ItemCommandBase.h"

void UUW_TempItem::BeginDestroy()
{
	if (Item.IsValid() == true)
	{
		Item->OnItemUpdated.RemoveAll(this);
	}

	Super::BeginDestroy();
}

void UUW_TempItem::SetItem(UItemInstance* InItem)
{
	if (IsValid(InItem) == false)
		return;

	Item = InItem;
	Item->OnItemUpdated.AddUObject(this, &ThisClass::UpdateTempItem);

	UpdateTempItem();
}

void UUW_TempItem::UpdateTempItem()
{
	PanelInteract->ClearChildren();

	if (Item.IsValid() == false)
	{
		TextItemId->SetText(FText::GetEmpty());
		return;
	}

	TextItemId->SetText(Item->GetData()->TextName);

	FItemCommandContext Ctx;
	Ctx.InstigatorController = GetOwningPlayer();
	if (Ctx.InstigatorController.IsValid())
	{
		Ctx.Instigator = Cast<AActor>(Ctx.InstigatorController->GetPawn());
	}
	const auto& Components = Item->GetItemComponents();
	TArray<TObjectPtr<UItemCommandBase>> Commands;
	for (const auto& Component : Components)
	{
		Component->GetCommands(Commands, Ctx);
	}

	for (const auto& Command : Commands)
	{
		UUW_TempInteract* NewInteractUI = CreateWidget<UUW_TempInteract>(this, InteractUIClass);
		if (IsValid(NewInteractUI) == false)
			continue;

		PanelInteract->AddChild(NewInteractUI);
		NewInteractUI->SetCommand(Command);
		NewInteractUI->OnExecuteCommand.BindUObject(this, &ThisClass::UpdateTempItem);
	}
}

void UUW_TempItem::SetHunger(float CurrentValue, float MaxValue)
{
	//TextHunger->SetText(FText::FromString(FString::Printf(TEXT("%f / %f"), CurrentValue, MaxValue)));
}

void UUW_TempItem::SetThirst(float CurrentValue, float MaxValue)
{
	//TextThirst->SetText(FText::FromString(FString::Printf(TEXT("%f / %f"), CurrentValue, MaxValue)));
}
