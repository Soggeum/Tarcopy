// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/UW_VoiceIndicator.h"

#include "Components/Image.h"
#include "UEOSVoiceChatSubsystem.h"

void UUW_VoiceIndicator::NativeConstruct()
{
	Super::NativeConstruct();

	SetVoiceIndicatorVisible(false);

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		VoiceSubsystem = GameInstance->GetSubsystem<UEOSVoiceChatSubsystem>();
		if (VoiceSubsystem)
		{
			VoiceSubsystem->OnVoiceTransmitStateChanged.AddDynamic(this, &ThisClass::HandleVoiceTransmitChanged);
			SetVoiceIndicatorVisible(VoiceSubsystem->IsVoiceIndicatorActive());
		}
	}
}

void UUW_VoiceIndicator::NativeDestruct()
{
	if (VoiceSubsystem)
	{
		VoiceSubsystem->OnVoiceTransmitStateChanged.RemoveDynamic(this, &ThisClass::HandleVoiceTransmitChanged);
	}

	Super::NativeDestruct();
}

void UUW_VoiceIndicator::HandleVoiceTransmitChanged(bool bIsActive)
{
	SetVoiceIndicatorVisible(bIsActive);
}

void UUW_VoiceIndicator::SetVoiceIndicatorVisible(bool bVisible)
{
	if (VoiceIcon)
	{
		VoiceIcon->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}
