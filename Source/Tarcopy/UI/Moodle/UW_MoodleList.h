// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MoodleList.generated.h"

class UUW_MoodleIcon;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_MoodleList : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_MoodleIcon> HungerIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUW_MoodleIcon> ThirstIcon;
};
