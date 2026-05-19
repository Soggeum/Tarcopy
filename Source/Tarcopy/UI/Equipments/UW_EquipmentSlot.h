// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_EquipmentSlot.generated.h"

class UItemInstance;
class UImage;
class USizeBox;
class UUW_ItemInfo;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_EquipmentSlot : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	void SetItem(UItemInstance* InItem);
	void SetSize(FVector2D NewSize);

private:
	void SetItemInfo();
	void OpenCommandMenu(const FPointerEvent& InMouseEvent);
	void UpdateIcon();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SlotSize;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img;

	UPROPERTY()
	TWeakObjectPtr<UItemInstance> Item;

	UPROPERTY()
	TObjectPtr<UUW_ItemInfo> Tooltip;

	UPROPERTY(EditDefaultsOnly, Category = "Tooltip")
	TSubclassOf<UUW_ItemInfo> TooltipClass;
};
