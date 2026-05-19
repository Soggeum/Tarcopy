// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"
#include "UW_FluidContainerInfo.generated.h"

class UTextBlock;
class UFluidContainerComponent;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_FluidContainerInfo : public UUW_ItemInfoEntry
{
	GENERATED_BODY()

public:
	virtual void BindItem(UItemInstance* InItem) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CapacityTxt;

	UPROPERTY()
	TWeakObjectPtr<UFluidContainerComponent> ChachedComponent;
};
