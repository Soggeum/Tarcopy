// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_NearbyPanel.h"

#include "Components/ScrollBox.h"
#include "Inventory/LootScannerComponent.h"
#include "Inventory/WorldContainerComponent.h"
#include "UI/UISubsystem.h"
#include "UI/Inventory/UW_Inventory.h"
#include "Components/NamedSlot.h"
#include "Components/Button.h"
#include "Blueprint/WidgetTree.h"
#include "Inventory/UW_ContainerBtn.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ContainerComponent.h"

void UUW_NearbyPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (!SelectedContainer)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}
	
	if (auto* LP = PC->GetLocalPlayer())
	{
		if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
		{
			InventoryWidget = CreateWidget<UUW_Inventory>(PC, InventoryWidgetClass);
			if (!InventoryWidget)
			{
				return;
			}

			SelectedContainer->SetContent(InventoryWidget);
			SelectedContainer->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}

void UUW_NearbyPanel::BindScanner(ULootScannerComponent* InScanner)
{
	if (!InScanner)
	{
		return;
	}

	BoundScanner = InScanner;
	BoundScanner->OnScannedContainersChanged.AddUObject(this, &UUW_NearbyPanel::RefreshContainerList);
	BoundScanner->OnScannedGroundChanged.AddUObject(this, &UUW_NearbyPanel::HandleGroundUpdatedWhileOpen);

	RefreshContainerList();
}

void UUW_NearbyPanel::RefreshContainerList()
{
	if (!BoundScanner || !ContainerScrollBox)
	{
		return;
	}

	TMap<UInventoryData*, FName> AvailableInv;

	for (const TWeakObjectPtr<UWorldContainerComponent>& C : BoundScanner->OverlappedContainers)
	{
		if (!IsValid(C.Get()))
		{
			continue;
		}
		if (UInventoryData* Inv = C->GetInventoryData())
		{
			AvailableInv.Add(Inv, C->GetContainerType());
		}
	}

	for (const TWeakObjectPtr<AItemWrapperActor>& W : BoundScanner->OverlappedContainerItems)
	{
		AItemWrapperActor* ItemActor = W.Get();
		if (!IsValid(ItemActor))
		{
			continue;
		}

		UItemInstance* Inst = ItemActor->GetItemInstance();
		if (!IsValid(Inst) || !Inst->GetData())
		{
			continue;
		}

		const UContainerComponent* ContainerComp = Inst->GetItemComponent<UContainerComponent>();
		if (ContainerComp)
		{
			if (UInventoryData* Inv = ContainerComp->GetInventoryData())
			{
				AvailableInv.Add(Inv, "Bag");
			}
		}
	}

	if (bInventoryPanelOpen && !bLastSelectedWasGround)
	{
		if (!AvailableInv.Contains(LastSelectedInventory))
		{
			SelectedContainer->SetVisibility(ESlateVisibility::Collapsed);
			bInventoryPanelOpen = false;
			LastSelectedInventory = nullptr;
		}
	}

	ContainerScrollBox->ClearChildren();

	for (const auto& Pair : AvailableInv)
	{
		UInventoryData* Inv = Pair.Key;
		UUW_ContainerBtn* Button = CreateWidget<UUW_ContainerBtn>(GetOwningPlayer(), ContainerBtnClass);
		Button->BindInventory(Inv, Pair.Value);
		Button->OnClickedWithInventory.AddUObject(this, &UUW_NearbyPanel::HandleContainerSelected);
		ContainerScrollBox->AddChild(Button);
	}

	UUW_ContainerBtn* GroundBtn = CreateWidget<UUW_ContainerBtn>(GetOwningPlayer(), ContainerBtnClass);
	GroundBtn->ContainerBtn->OnClicked.AddDynamic(this, &UUW_NearbyPanel::HandleGroundSelected);
	GroundBtn->RefreshVisual("Ground");
	ContainerScrollBox->AddChild(GroundBtn);
}

void UUW_NearbyPanel::HandleContainerSelected(UInventoryData* Inventory)
{
	if (!IsValid(Inventory) || !InventoryWidget)
	{
		return;
	}

	const bool bSameAsLast = bInventoryPanelOpen && !bLastSelectedWasGround && LastSelectedInventory == Inventory;
	if (bSameAsLast)
	{
		SelectedContainer->SetVisibility(ESlateVisibility::Collapsed);
		bInventoryPanelOpen = false;
		bLastSelectedWasGround = false;
		LastSelectedInventory = nullptr;
		return;
	}

	SelectedContainer->SetVisibility(ESlateVisibility::Visible);
	InventoryWidget->BindInventory(Inventory);

	bInventoryPanelOpen = true;
	bLastSelectedWasGround = false;
	LastSelectedInventory = Inventory;
}

void UUW_NearbyPanel::HandleGroundSelected()
{
	if (!BoundScanner || !InventoryWidget)
	{
		return;
	}

	const bool bSameAsLast = bInventoryPanelOpen && bLastSelectedWasGround;
	if (bSameAsLast)
	{
		SelectedContainer->SetVisibility(ESlateVisibility::Collapsed);
		bInventoryPanelOpen = false;
		bLastSelectedWasGround = false;
		LastSelectedInventory = nullptr;
		return;
	}

	SelectedContainer->SetVisibility(ESlateVisibility::Visible);
	InventoryWidget->BindInventory(BoundScanner->GetGroundInventoryData());

	bInventoryPanelOpen = true;
	bLastSelectedWasGround = true;
	LastSelectedInventory = nullptr;
}

void UUW_NearbyPanel::HandleGroundUpdatedWhileOpen()
{
	if (!bInventoryPanelOpen || !bLastSelectedWasGround)
	{
		return;
	}

	if (!BoundScanner || !InventoryWidget)
	{
		return;
	}

	InventoryWidget->BindInventory(BoundScanner->GetGroundInventoryData());
}
