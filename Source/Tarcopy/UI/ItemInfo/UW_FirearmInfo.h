// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"
#include "UW_FirearmInfo.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_FirearmInfo : public UUW_ItemInfoEntry
{
	GENERATED_BODY()

public:
	virtual void BindItem(UItemInstance* InItem) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BodyLocationTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RangeTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> AccurancyTxt;
};
