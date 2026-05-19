// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_InventoryBorder.h"

#include "Components/NamedSlot.h"
#include "Components/Button.h"
#include "UI/UISubsystem.h"
#include "Inventory/InventoryData.h"

void UUW_InventoryBorder::NativeConstruct()
{
    Super::NativeConstruct();

    if (CloseBtn)
    {
        CloseBtn->OnClicked.RemoveAll(this);
        CloseBtn->OnClicked.AddDynamic(this, &UUW_InventoryBorder::OnClickClose);
    }
}

void UUW_InventoryBorder::SetContentWidget(UWidget* InWidget)
{
	ContentSlot->SetContent(InWidget);
}

UWidget* UUW_InventoryBorder::GetContentWidget() const
{
	return ContentSlot->GetContent();
}

void UUW_InventoryBorder::OnClickClose()
{
    UInventoryData* Inv = InventoryData.Get();
    if (!IsValid(Inv))
    {
        RemoveFromParent();
        return;
    }

    if (ULocalPlayer* LP = GetOwningLocalPlayer())
    {
        if (UUISubsystem* UISub = LP->GetSubsystem<UUISubsystem>())
        {
            UISub->HideInventoryUI(Inv);
            return;
        }
    }

    RemoveFromParent();
}
