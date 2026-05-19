// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_Inventory.generated.h"

class UInventoryData;
class UUW_InventoryItem;
class UUW_InventoryCell;
class UUniformGridPanel;
class UCanvasPanel;
class UInventoryDragDropOp;
class UItemInstance;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_Inventory : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	virtual void NativeDestruct() override;

public:
	void BindInventory(UInventoryData* InData);

	void AddItemWidget(UItemInstance* Item, const FIntPoint& Origin, bool bRotated);

	void RefreshItems();

	int32 GetCellSizePx() const { return CellSizePx; }

	void ForceUpdatePreviewFromOp(UInventoryDragDropOp* Op);

private:
	void BuildGrid(FIntPoint GridSize);
	void ClearGrid();
	void BuildItems();
	UUW_InventoryCell* GetCell(int32 X, int32 Y) const;
	int32 ToIndex(int32 X, int32 Y) const;

	void ClearCellPreview();
	void ApplyCellPreview(const FIntPoint& Origin, const FIntPoint& Size, const FLinearColor& Color);


	const int32 CellSizePx = 75;

	UPROPERTY()
	TObjectPtr<UInventoryData> BoundInventory;

	UPROPERTY()
	TMap<TWeakObjectPtr<UItemInstance>, TObjectPtr<UUW_InventoryItem>> ItemWidgets;


	UPROPERTY(EditDefaultsOnly, Category = "Inventory|UI")
	TSubclassOf<UUW_InventoryCell> CellWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory|UI")
	TSubclassOf<UUW_InventoryItem> ItemWidgetClass;


	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ItemCanvas;

	UPROPERTY()
	TArray<TObjectPtr<UUW_InventoryCell>> CellWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UUW_InventoryCell>> PreviewCells;

	FIntPoint CachedGridSize = FIntPoint::ZeroValue;

};
