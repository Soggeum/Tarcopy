// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "InventoryDragDropOp.generated.h"

class UInventoryData;
class UUW_Inventory;
class USizeBox;
class UItemInstance;
class AItemWrapperActor;

/**
 * 
 */
UCLASS()
class TARCOPY_API UInventoryDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	virtual void DragCancelled_Implementation(const FPointerEvent& PointerEvent) override;

public:
	UPROPERTY()
	TWeakObjectPtr<UItemInstance> Item;

	UPROPERTY()
	TWeakObjectPtr<AItemWrapperActor> SourceWorldActor;

	UPROPERTY()
	TObjectPtr<UInventoryData> SourceInventory;

	UPROPERTY()
	TObjectPtr<UUW_Inventory> SourceInventoryWidget;

	UPROPERTY()
	FVector2D GrabOffsetPx = FVector2D::ZeroVector;

	UPROPERTY()
	TObjectPtr<USizeBox> DragBox;

	UPROPERTY()
	TWeakObjectPtr<UUW_Inventory> HoveredInventoryWidget;

	UPROPERTY()
	FVector2D LastScreenPos;

	UPROPERTY()
	uint8 bRotated : 1 = false;
};
