// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipments/UW_EquipmentSlot.h"

#include "Item/ItemInstance.h"
#include "Components/SizeBox.h"
#include "UI/ItemInfo/UW_ItemInfo.h"
#include "UI/UISubsystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Item/Data/ItemData.h"

FReply UUW_EquipmentSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OpenCommandMenu(InMouseEvent);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUW_EquipmentSlot::SetItem(UItemInstance* InItem)
{
    Item = InItem;
    SetItemInfo();
	UpdateIcon();
}

void UUW_EquipmentSlot::SetSize(FVector2D NewSize)
{
    SlotSize->SetWidthOverride(NewSize.X);
    SlotSize->SetHeightOverride(NewSize.Y);
}

void UUW_EquipmentSlot::SetItemInfo()
{
	if (!TooltipClass || !Item.IsValid())
	{
		return;
	}
	Tooltip = CreateWidget<UUW_ItemInfo>(GetOwningPlayer(), TooltipClass);
	Tooltip->BindItem(Item.Get());
	SetToolTip(Tooltip);
}

void UUW_EquipmentSlot::OpenCommandMenu(const FPointerEvent& InMouseEvent)
{
	UItemInstance* ItemInst = Item.Get();
	if (!IsValid(ItemInst))
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

	const FVector2D ViewportPos = UWidgetLayoutLibrary::GetMousePositionOnViewport(PC);
	UIS->ShowItemCommandMenu(ItemInst, ViewportPos);
}

void UUW_EquipmentSlot::UpdateIcon()
{
	if (!Img)
	{
		return;
	}

	UItemInstance* ItemInst = Item.Get();
	const FItemData* Data = ItemInst ? ItemInst->GetData() : nullptr;

	if (!Data || !Data->ItemIcon)
	{
		return;
	}

	Img->SetBrushFromTexture(Data->ItemIcon, true);

	Img->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
}