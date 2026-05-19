// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ItemInfo.generated.h"

class UItemInstance;
class UVerticalBox;
class UItemInfoConfig;
class UUW_ItemInfoEntry;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ItemInfo : public UUserWidget
{
	GENERATED_BODY()

public:
	void BindItem(UItemInstance* Item);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VBox;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UItemInfoConfig> Config;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_ItemInfoEntry> DefaultInfoClass;
};
