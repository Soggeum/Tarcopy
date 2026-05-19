// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/UW_ContainerBtn.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Inventory/InventoryData.h"

void UUW_ContainerBtn::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UUW_ContainerBtn::BindInventory(UInventoryData* InInventory, FName Type)
{
	Inventory = InInventory;
	if (Type != "Ground")
	{
		ContainerBtn->OnClicked.AddDynamic(this, &UUW_ContainerBtn::HandleClicked);
	}
	RefreshVisual(Type);
}

void UUW_ContainerBtn::RefreshVisual(FName Type)
{
	if (!ContainerImg)
	{
		return;
	}

	if (Type == TEXT("Ground"))
	{
		ContainerImg->SetBrushFromTexture(Icon_Ground, true);
	}
	else if (Type == TEXT("Box"))
	{
		ContainerImg->SetBrushFromTexture(Icon_Box, true);
	}
	else if (Type == TEXT("Bag"))
	{
		ContainerImg->SetBrushFromTexture(Icon_Bag, true);
	}
	else if (Type == TEXT("Zombie"))
	{
		ContainerImg->SetBrushFromTexture(Icon_Zombie, true);
	}
	else
	{
		ContainerImg->SetBrushFromTexture(Icon_Box, true);
	}
}

void UUW_ContainerBtn::HandleClicked()
{
	if (!IsValid(Inventory))
	{
		return;
	}
	OnClickedWithInventory.Broadcast(Inventory);
}
