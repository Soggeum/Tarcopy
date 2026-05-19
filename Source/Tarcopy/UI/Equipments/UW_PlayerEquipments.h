// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_PlayerEquipments.generated.h"

class UEquipComponent;
class UUW_SlotsByPart;
enum class EBodyLocation : uint32;
/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_PlayerEquipments : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

public:
	void BindEquipComponent(UEquipComponent* InEquipComp);

	UFUNCTION()
	void SetSlotByPart();

private:
	UUW_SlotsByPart* GetGroupWidgetByLocation(EBodyLocation Loc) const;

private:
	UPROPERTY()
	TObjectPtr<UEquipComponent> BoundEquipComp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> HeadSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> BackSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> TopSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> BottomSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> ShoeSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> RightWristSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> LeftWristSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> RightHandSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_SlotsByPart> LeftHandSlot;
};
