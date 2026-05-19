// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_Inventory.h"

#include "Inventory/InventoryData.h"
#include "UI/Inventory/UW_InventoryCell.h"
#include "UI/Inventory/UW_InventoryItem.h"
#include "Components/UniformGridPanel.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Item/ItemInstance.h"
#include "UI/Inventory/InventoryDragDropOp.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/LootScannerComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"

bool UUW_Inventory::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	ClearCellPreview();

	if (!BoundInventory || !InOperation)
	{
		return false;
	}

	UInventoryDragDropOp* Op = Cast<UInventoryDragDropOp>(InOperation);
	if (!Op || !IsValid(Op->SourceInventory))
	{
		return false;
	}

	const UInventoryData* Ground = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* P = PC->GetPawn())
		{
			if (ULootScannerComponent* Scanner = P->FindComponentByClass<ULootScannerComponent>())
			{
				Ground = Scanner->GetGroundInventoryData();
				if (Ground && Op->SourceInventory == Ground && BoundInventory == Ground)
				{
					return false;
				}
			}
		}
	}

	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());

	const FVector2D TopLeftPx = LocalPos - Op->GrabOffsetPx;

	const int32 NewX = FMath::FloorToInt(TopLeftPx.X / CellSizePx);
	const int32 NewY = FMath::FloorToInt(TopLeftPx.Y / CellSizePx);
	const FIntPoint NewOrigin(NewX, NewY);

	UItemInstance* Item = Op->Item.Get();
	if (!IsValid(Item))
	{
		return false;
	}

	UPlayerInventoryComponent* InvComp = nullptr;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* P = PC->GetPawn())
		{
			InvComp = P->FindComponentByClass<UPlayerInventoryComponent>();
		}
	}
	if (!InvComp)
	{
		return false;
	}

	if (Op->SourceInventory == Ground)
	{
		AItemWrapperActor* WorldActor = Op->SourceWorldActor.Get();
		if (!IsValid(WorldActor))
		{
			return false;
		}

		InvComp->RequestLootFromWorld(WorldActor, BoundInventory, NewOrigin, Op->bRotated);
		return true;
	}

	InvComp->RequestMoveItem(Op->SourceInventory, Item, BoundInventory, NewOrigin, Op->bRotated);

	return true;
}

bool UUW_Inventory::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	if (!BoundInventory || !InOperation)
	{
		ClearCellPreview();
		return false;
	}

	UInventoryDragDropOp* Op = Cast<UInventoryDragDropOp>(InOperation);
	if (!Op || !IsValid(Op->SourceInventory))
	{
		ClearCellPreview();
		return false;
	}

	Op->HoveredInventoryWidget = this;
	Op->LastScreenPos = InDragDropEvent.GetScreenSpacePosition();

	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	const FVector2D TopLeftPx = LocalPos - Op->GrabOffsetPx;

	const int32 NewX = FMath::FloorToInt(TopLeftPx.X / CellSizePx);
	const int32 NewY = FMath::FloorToInt(TopLeftPx.Y / CellSizePx);
	const FIntPoint Origin(NewX, NewY);

	ClearCellPreview();

	UItemInstance* Item = Op->Item.Get();
	if (!IsValid(Item))
	{
		ClearCellPreview();
		return false;
	}

	const FIntPoint ItemSize = BoundInventory->GetItemSize(Item, Op->bRotated);
	if (ItemSize == FIntPoint::ZeroValue)
	{
		ClearCellPreview();
		return false;
	}

	const bool bCanPlace = BoundInventory->CanPlaceItemPreview(
		Item,
		Op->SourceInventory,
		Origin,
		Op->bRotated
	);

	const FLinearColor Color = bCanPlace ? FLinearColor(0.f, 1.f, 0.f, 0.35f) : FLinearColor(1.f, 0.f, 0.f, 0.35f);

	ApplyCellPreview(Origin, ItemSize, Color);

	return true;
}

void UUW_Inventory::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (UInventoryDragDropOp* Op = Cast<UInventoryDragDropOp>(InOperation))
	{
		if (Op->HoveredInventoryWidget.Get() == this)
		{
			Op->HoveredInventoryWidget = nullptr;
		}
	}

	ClearCellPreview();
}

void UUW_Inventory::NativeDestruct()
{
	if (BoundInventory)
	{
		BoundInventory->OnInventoryChanged.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UUW_Inventory::BindInventory(UInventoryData* InData)
{
	if (BoundInventory)
	{
		BoundInventory->OnInventoryChanged.RemoveAll(this);
	}

	BoundInventory = InData;

	ClearGrid();
	if (!BoundInventory)
	{
		return;
	}

	BoundInventory->OnInventoryChanged.AddUObject(this, &UUW_Inventory::RefreshItems);
	UE_LOG(LogTemp, Warning, TEXT("[UI] BindInventory this=%p Inv=%s (%p)"),
		this, *GetNameSafe(InData), InData);

	BuildGrid(BoundInventory->GetGridSize());
	BuildItems();
}

void UUW_Inventory::AddItemWidget(UItemInstance* Item, const FIntPoint& Origin, bool bRotated)
{
	if (!IsValid(Item) || !BoundInventory)
	{
		return;
	}

	UUW_InventoryItem* ItemWidget = CreateWidget<UUW_InventoryItem>(GetOwningPlayer(), ItemWidgetClass);
	UE_LOG(LogTemp, Warning, TEXT("[UI] Created ItemWidget=%s Item=%s"),
		*GetNameSafe(ItemWidget), *GetNameSafe(Item));

	if (!ItemWidget)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(ItemWidget);
	CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
	CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));

	const FIntPoint ItemSize = BoundInventory->GetItemSize(Item, bRotated);

	const FVector2D PosPx(Origin.X * CellSizePx, Origin.Y * CellSizePx);
	const FVector2D SizePx(ItemSize.X * CellSizePx, ItemSize.Y * CellSizePx);

	CanvasSlot->SetPosition(PosPx);
	CanvasSlot->SetSize(SizePx);

	ItemWidget->InitItem(Item, BoundInventory, this, bRotated);

	ItemWidgets.Add(Item, ItemWidget);
}

void UUW_Inventory::RefreshItems()
{
	if (!BoundInventory)
	{
		return;
	}

	if (ItemCanvas)
	{
		ItemCanvas->ClearChildren();
	}
	ItemWidgets.Empty();

	BuildItems();

	UE_LOG(LogTemp, Warning, TEXT("[UI] RefreshItems Inv=%s Entries=%d"),
		*GetNameSafe(BoundInventory), BoundInventory->GetReplicatedItems().Items.Num());

	for (const auto& Entry : BoundInventory->GetReplicatedItems().Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("[UI] Entry Item=%s Origin=%s"),
			*GetNameSafe(Entry.Item), *Entry.Origin.ToString());
	}
}

void UUW_Inventory::ForceUpdatePreviewFromOp(UInventoryDragDropOp* Op)
{
	if (!BoundInventory || !Op || !IsValid(Op->SourceInventory))
	{
		ClearCellPreview();
		return;
	}
	const FVector2D LocalPos = GetCachedGeometry().AbsoluteToLocal(Op->LastScreenPos);
	const FVector2D TopLeftPx = LocalPos - Op->GrabOffsetPx;

	const int32 NewX = FMath::FloorToInt(TopLeftPx.X / CellSizePx);
	const int32 NewY = FMath::FloorToInt(TopLeftPx.Y / CellSizePx);
	const FIntPoint Origin(NewX, NewY);

	ClearCellPreview();
	UItemInstance* Item = Op->Item.Get();
	if (!IsValid(Item))
	{
		ClearCellPreview();
		return;
	}

	const FIntPoint ItemSize = BoundInventory->GetItemSize(Item, Op->bRotated);
	if (ItemSize == FIntPoint::ZeroValue)
	{
		ClearCellPreview();
		return;
	}

	const bool bCanPlace = BoundInventory->CanPlaceItemPreview(
		Item,
		Op->SourceInventory,
		Origin,
		Op->bRotated
	);

	const FLinearColor Color = bCanPlace
		? FLinearColor(0.f, 1.f, 0.f, 0.35f)
		: FLinearColor(1.f, 0.f, 0.f, 0.35f);

	ApplyCellPreview(Origin, ItemSize, Color);
}

void UUW_Inventory::BuildGrid(FIntPoint GridSize)
{
	CachedGridSize = GridSize;
	CellWidgets.SetNum(GridSize.X * GridSize.Y);

	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			UUW_InventoryCell* Cell = CreateWidget<UUW_InventoryCell>(GetOwningPlayer(), CellWidgetClass);
			GridPanel->AddChildToUniformGrid(Cell, Y, X);

			CellWidgets[ToIndex(X, Y)] = Cell;
		}
	}
}

void UUW_Inventory::ClearGrid()
{
	if (GridPanel)
	{
		GridPanel->ClearChildren();
	}

	if (ItemCanvas)
	{
		ItemCanvas->ClearChildren();
	}

	CellWidgets.Empty();
	PreviewCells.Empty();
	ItemWidgets.Empty();
}

void UUW_Inventory::BuildItems()
{
	if (!BoundInventory || !ItemCanvas)
	{
		return;
	}

	ItemCanvas->ClearChildren();
	ItemWidgets.Empty();

	const FInventoryItemList& RepList = BoundInventory->GetReplicatedItems();
	UE_LOG(LogTemp, Warning, TEXT("[UI] BuildItems Inv=%s Num=%d"),
		*GetNameSafe(BoundInventory), RepList.Items.Num());

	int32 NullCount = 0;
	for (const FInventoryItemEntry& Entry : RepList.Items)
	{
		if (!IsValid(Entry.Item))
		{
			++NullCount;
			continue;
		}
		UE_LOG(LogTemp, Warning, TEXT("[UI] AddItem Try Item=%s Size=%s"),
			*GetNameSafe(Entry.Item),
			*BoundInventory->GetItemSize(Entry.Item, Entry.bRotated).ToString());

		AddItemWidget(Entry.Item, Entry.Origin, Entry.bRotated);
	}
	UE_LOG(LogTemp, Warning, TEXT("[UI] BuildItems NullItem=%d"), NullCount);
}


UUW_InventoryCell* UUW_Inventory::GetCell(int32 X, int32 Y) const
{
	if (X < 0 || Y < 0 || X >= CachedGridSize.X || Y >= CachedGridSize.Y)
	{
		return nullptr;
	}
	const int32 Idx = ToIndex(X, Y);
	return CellWidgets.IsValidIndex(Idx) ? CellWidgets[Idx].Get() : nullptr;
}

int32 UUW_Inventory::ToIndex(int32 X, int32 Y) const
{
	return Y * CachedGridSize.X + X;
}

void UUW_Inventory::ClearCellPreview()
{
	for (TObjectPtr<UUW_InventoryCell>& Cell : PreviewCells)
	{
		if (Cell)
		{
			Cell->ClearPreview();
		}
	}
	PreviewCells.Empty();
}

void UUW_Inventory::ApplyCellPreview(const FIntPoint& Origin, const FIntPoint& Size, const FLinearColor& Color)
{
	for (int32 Y = 0; Y < Size.Y; ++Y)
	{
		for (int32 X = 0; X < Size.X; ++X)
		{
			UUW_InventoryCell* Cell = GetCell(Origin.X + X, Origin.Y + Y);
			if (Cell)
			{
				Cell->SetPreviewColor(Color);
				PreviewCells.Add(Cell);
			}
		}
	}
}