// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_ItemCommandMenu.h"

#include "Components/VerticalBox.h"
#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "UI/Inventory/UW_ItemCommandEntry.h"

void UUW_ItemCommandMenu::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_ItemCommandMenu::InitMenu(UItemInstance* InItem)
{
	Item = InItem;
	Commands.Reset();

	if (!Item.IsValid())
	{
		return;
	}

	FItemCommandContext Ctx;
	Ctx.InstigatorController = GetOwningPlayer();
	if (Ctx.InstigatorController.IsValid())
	{
		Ctx.Instigator = Cast<AActor>(Ctx.InstigatorController->GetPawn());
	}

	const auto Components = Item->GetItemComponents();
	for (const auto& Comp : Components)
	{
		if (IsValid(Comp))
		{
			Comp->GetCommands(Commands, Ctx);
		}
	}

	bInitialized = true;

	RebuildEntries();
}

void UUW_ItemCommandMenu::RebuildEntries()
{
	UE_LOG(LogTemp, Warning, TEXT("[CmdMenu] RebuildEntries Commands=%d Panel=%s"),
		Commands.Num(), *GetNameSafe(PanelEntries));

	if (!PanelEntries)
	{
		return;
	}

	PanelEntries->ClearChildren();

	for (UItemCommandBase* Cmd : Commands)
	{
		if (!bInitialized)
		{
			return;
		}

		if (!IsValid(Cmd) || !EntryClass)
		{
			continue;
		}

		UUW_ItemCommandEntry* Entry = CreateWidget<UUW_ItemCommandEntry>(GetOwningPlayer(), EntryClass);
		if (!IsValid(Entry))
		{
			continue;
		}

		PanelEntries->AddChildToVerticalBox(Entry);

		Entry->InitEntry(Cmd);
		Entry->OnExecuted.AddUObject(this, &ThisClass::HandleEntryExecuted);


		UE_LOG(LogTemp, Warning, TEXT("[CmdMenu] Add Entry=%s Cmd=%s"),
			*GetNameSafe(Entry), *GetNameSafe(Cmd));
	}
}

void UUW_ItemCommandMenu::HandleEntryExecuted()
{
	RemoveFromParent();
}
