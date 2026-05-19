// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ItemInfoEntry.generated.h"

class UItemInstance;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ItemInfoEntry : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void BindItem(UItemInstance* InItem);
	virtual void HandleUpdated();
};
