// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/Inventory/UW_Inventory.h"

#include "UW_InventoryItem.generated.h"

class UInventoryData;
class UUW_Inventory;
class UBorder;
class UItemInstance;
class UUW_ItemInfo;
class UImage;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_InventoryItem : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

public:
	void InitItem(UItemInstance* InItem, UInventoryData* InSourceInventory, UUW_Inventory* InSourceWidget, bool bInRotated);

private:
	void ApplyProxyVisual();
	FVector2D GetItemPixelSize() const;

	void OpenCommandMenu(const FPointerEvent& InMouseEvent);

	void SetItemInfo();

	void UpdateIcon();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> ItemBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> ItemBG;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img;

	UPROPERTY()
	TWeakObjectPtr<UItemInstance> Item;

	UPROPERTY()
	TObjectPtr<UInventoryData> SourceInventory;

	UPROPERTY()
	TObjectPtr<UUW_Inventory> SourceInventoryWidget;

	UPROPERTY()
	TObjectPtr<UUW_ItemInfo> Tooltip;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UUW_ItemInfo> TooltipClass;

	UPROPERTY()
	bool bRotated = false;
};
