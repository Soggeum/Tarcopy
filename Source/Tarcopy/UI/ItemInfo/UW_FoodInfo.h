// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"
#include "UW_FoodInfo.generated.h"

class UTextBlock;
class UFoodComponent;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_FoodInfo : public UUW_ItemInfoEntry
{
	GENERATED_BODY()

public:
	virtual void BindItem(UItemInstance* InItem) override;
	virtual void HandleUpdated() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HungerTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ThirstTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RemainAmountTxt;

	UPROPERTY()
	TWeakObjectPtr<UFoodComponent> CachedComponent;
};
