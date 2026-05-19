// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_VoiceIndicator.generated.h"

class UImage;
class UEOSVoiceChatSubsystem;

UCLASS()
class TARCOPY_API UUW_VoiceIndicator : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleVoiceTransmitChanged(bool bIsActive);

	void SetVoiceIndicatorVisible(bool bVisible);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> VoiceIcon;

	UPROPERTY()
	TObjectPtr<UEOSVoiceChatSubsystem> VoiceSubsystem;
};
