// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_PlayerPanel.h"

#include "Components/ScrollBox.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/InventoryData.h"
#include "UI/UISubsystem.h"
#include "UI/Inventory/UW_Inventory.h"
#include "UI/Equipments/UW_PlayerEquipments.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

void UUW_PlayerPanel::NativeConstruct()
{
	Super::NativeConstruct();
}

void UUW_PlayerPanel::BindPlayerInventory(UPlayerInventoryComponent* InComp)
{
	if (!IsValid(InComp))
	{
		return;
	}

	if (BoundComp)
	{
		BoundComp->OnInventoryReady.RemoveAll(this);
	}

	BoundComp = InComp;
	BoundComp->OnInventoryReady.AddUObject(this, &UUW_PlayerPanel::HandleInventoryReady);

	RefreshInventories();
}

void UUW_PlayerPanel::RefreshInventories()
{
	if (!BoundComp || !InventoryScrollBox)
	{
		return;
	}

	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}

	ULocalPlayer* LP = PC->GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	UUISubsystem* UIS = LP->GetSubsystem<UUISubsystem>();
	if (!UIS)
	{
		return;
	}

	UInventoryData* Data = BoundComp->GetPlayerInventoryData();
	if (!Data)
	{
		return;
	}

	if(!IsValid(PlayerInventoryWidget))
	{
		if (!InventoryWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("UUW_PlayerPanel: InventoryWidgetClass is null"));
			return;
		}

		PlayerInventoryWidget = CreateWidget<UUW_Inventory>(PC, InventoryWidgetClass);
		if (!IsValid(PlayerInventoryWidget))
		{
			return;
		}

		InventoryScrollBox->ClearChildren();
		InventoryScrollBox->AddChild(PlayerInventoryWidget);
	}

	PlayerInventoryWidget->BindInventory(Data);
	Data->ForceRefreshNextTick();
}

void UUW_PlayerPanel::BindEquipComponent(UEquipComponent* InComp)
{
	PlayerEquipmentsWidget->BindEquipComponent(InComp);
}

void UUW_PlayerPanel::HandleInventoryReady()
{
	RefreshInventories();
}
