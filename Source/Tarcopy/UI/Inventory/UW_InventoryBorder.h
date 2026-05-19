// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/InventoryData.h"
#include "UW_InventoryBorder.generated.h"

class UTextBlock;
class UButton;
class UNamedSlot;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_InventoryBorder : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void SetContentWidget(UWidget* InWidget);
	UWidget* GetContentWidget() const;

	void SetInventoryData(UInventoryData* InData) { InventoryData = InData; }
	
private:
	UFUNCTION()
	void OnClickClose();

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UTextBlock> ContainerName;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UButton> CloseBtn;

	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UNamedSlot> ContentSlot;

	TWeakObjectPtr<UInventoryData> InventoryData;
	
};
