// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_SlotsByPart.generated.h"

class UNamedSlot;
class UScrollBox;
class UItemInstance;
struct FEquippedItemInfo;
class UUW_EquipmentSlot;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_SlotsByPart : public UUserWidget
{
	GENERATED_BODY()

public:
	void AddItem(FEquippedItemInfo Info);
	void ClearItems() { Items.Empty(); }
	void RefreshSlot();
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNamedSlot> MainSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> SubSlots;

	UPROPERTY()
	TArray<FEquippedItemInfo> Items;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class UUW_EquipmentSlot> EquipmentSlotClass;

	UPROPERTY(EditDefaultsOnly)
	FVector2D MainSize = FVector2D(80.f, 80.f);

	UPROPERTY(EditDefaultsOnly)
	FVector2D SubSize = FVector2D(20.f, 20.f);
};
