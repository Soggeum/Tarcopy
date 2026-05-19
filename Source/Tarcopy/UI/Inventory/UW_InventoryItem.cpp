// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_InventoryItem.h"

#include "UI/Inventory/UW_Inventory.h"
#include "UI/Inventory/InventoryDragDropOp.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Inventory/InventoryData.h"
#include "Components/SizeBox.h"
#include "Components/Border.h"
#include "UI/Inventory/UW_ItemCommandMenu.h"
#include "Item/ItemInstance.h"
#include "UI/UISubsystem.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Inventory/LootScannerComponent.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "UI/ItemInfo/UW_ItemInfo.h"
#include "Components/Image.h"
#include "Item/Data/ItemData.h"

void UUW_InventoryItem::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UUW_InventoryItem::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		OpenCommandMenu(InMouseEvent);
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UUW_InventoryItem::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UInventoryDragDropOp* Op = NewObject<UInventoryDragDropOp>(GetOwningPlayer());
	Op->Item = Item;
	Op->SourceInventory = SourceInventory;
	Op->SourceInventoryWidget = SourceInventoryWidget;
	Op->bRotated = bRotated;
	Op->GrabOffsetPx = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* P = PC->GetPawn())
		{
			if (ULootScannerComponent* Scanner = P->FindComponentByClass<ULootScannerComponent>())
			{
				if (SourceInventory == Scanner->GetGroundInventoryData())
				{
					Op->SourceWorldActor = MakeWeakObjectPtr(Scanner->FindWorldActorByItem(Item.Get()));
				}
			}
		}
	}

	UUW_InventoryItem* Proxy = CreateWidget<UUW_InventoryItem>(GetOwningPlayer(), GetClass());
	if (Proxy)
	{
		Proxy->InitItem(Item.Get(), SourceInventory, SourceInventoryWidget, bRotated);
		Proxy->ApplyProxyVisual();
		Proxy->SetRenderOpacity(0.4f);

		const FVector2D SizePx = GetItemPixelSize();

		USizeBox* Box = NewObject<USizeBox>(this);
		Op->DragBox = Box;
		Box->SetWidthOverride(SizePx.X);
		Box->SetHeightOverride(SizePx.Y);
		Box->AddChild(Proxy);

		Op->DefaultDragVisual = Box;
	}
	else
	{
		Op->DefaultDragVisual = this;
	}

	Op->Pivot = EDragPivot::MouseDown;

	OutOperation = Op;
}

void UUW_InventoryItem::InitItem(UItemInstance* InItem, UInventoryData* InSourceInventory, UUW_Inventory* InSourceWidget, bool bInRotated)
{
	Item = InItem;
	SourceInventory = InSourceInventory;
	SourceInventoryWidget = InSourceWidget;
	bRotated = bInRotated;

	SetItemInfo();
	UpdateIcon();
}

void UUW_InventoryItem::ApplyProxyVisual()
{
	if (ItemBorder)
	{
		ItemBorder->SetBrushColor(FLinearColor(0, 0, 0, 0));
	}
	if (ItemBG)
	{
		ItemBG->SetBrushColor(FLinearColor(0, 0, 0, 0));
	}
}

FVector2D UUW_InventoryItem::GetItemPixelSize() const
{
	if (!IsValid(SourceInventory) || !IsValid(SourceInventoryWidget))
	{
		return FVector2D::ZeroVector;
	}

	const int32 CellPx = SourceInventoryWidget->GetCellSizePx();
	const FIntPoint SizeCells = SourceInventory->GetItemSize(Item.Get(), bRotated);

	return FVector2D(SizeCells.X * CellPx, SizeCells.Y * CellPx);
}

void UUW_InventoryItem::OpenCommandMenu(const FPointerEvent& InMouseEvent)
{
	if (!IsValid(SourceInventory))
	{
		return;
	}

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

void UUW_InventoryItem::SetItemInfo()
{
	if (!TooltipClass || !Item.IsValid())
	{
		return;
	}
	Tooltip = CreateWidget<UUW_ItemInfo>(GetOwningPlayer(), TooltipClass);
	Tooltip->BindItem(Item.Get());
	SetToolTip(Tooltip);
}

void UUW_InventoryItem::UpdateIcon()
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

	Img->SetRenderTransformAngle(bRotated ? 90.0f : 0.0f);
	Img->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
}