// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"
#include "UW_DurabilityInfo.generated.h"

class UTextBlock;
class UDurabilityComponent;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_DurabilityInfo : public UUW_ItemInfoEntry
{
	GENERATED_BODY()

public:
	virtual void BindItem(UItemInstance* InItem) override;
	virtual void HandleUpdated() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ConditionTxt;

	UPROPERTY()
	TWeakObjectPtr<UDurabilityComponent> CachedComponent;
};
