// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_NearbyPanel.generated.h"

class ULootScannerComponent;
class UNamedSlot;
class UUW_Inventory;
class UScrollBox;
class UUW_ContainerBtn;
class UInventoryData;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_NearbyPanel : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void BindScanner(ULootScannerComponent* InScanner);

private:
	void RefreshContainerList();

	UFUNCTION()
	void HandleContainerSelected(UInventoryData* Inventory);

	UFUNCTION()
	void HandleGroundSelected();

	UFUNCTION()
	void HandleGroundUpdatedWhileOpen();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> SelectedContainer;

	UPROPERTY()
	TObjectPtr<UInventoryData> LastSelectedInventory;

	UPROPERTY()
	bool bLastSelectedWasGround = false;

	UPROPERTY()
	bool bInventoryPanelOpen = false;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_ContainerBtn> ContainerBtnClass;

	UPROPERTY()
	TObjectPtr<UUW_Inventory> InventoryWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ContainerScrollBox;

	UPROPERTY()
	TObjectPtr<ULootScannerComponent> BoundScanner;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_Inventory> InventoryWidgetClass;
};
