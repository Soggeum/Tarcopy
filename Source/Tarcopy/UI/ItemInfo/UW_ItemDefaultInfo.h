// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"
#include "UW_ItemDefaultInfo.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ItemDefaultInfo : public UUW_ItemInfoEntry
{
	GENERATED_BODY()

public:
	virtual void BindItem(UItemInstance* InItem) override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextNameTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ItemTypeTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextDescTxt;
};
