// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "ItemInfoConfig.generated.h"

/**
 * 
 */
UCLASS()
class TARCOPY_API UItemInfoConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	bool GetInfo(TSubclassOf<UItemComponentBase> Type, TSubclassOf<UUserWidget>& OutInfo) const;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<TSubclassOf<UItemComponentBase>, TSubclassOf<UUserWidget>> InfoConfig;
};
