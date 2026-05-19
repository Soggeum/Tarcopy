// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_MoodleIcon.generated.h"

class UProgressBar;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_MoodleIcon : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetRatio(float InRatio);

private:
	FLinearColor GetMoodColor(float Ratio);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BackgroundFill;
};
