// Fill out your copyright notice in the Description page of Project Settings.

#include "Settings/TarcopySettingsSubsystem.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

UTarcopySettingsSubsystem::UTarcopySettingsSubsystem()
	: MasterSoundMix(nullptr)
	, MasterSoundClass(nullptr)
	, MusicSoundClass(nullptr)
	, SfxSoundClass(nullptr)
	, MasterVolume(1.0f)
	, MusicVolume(1.0f)
	, SfxVolume(1.0f)
{
}

void UTarcopySettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ApplySoundMix();
}

static EWindowMode::Type ToWindowMode(ETarcopyWindowMode InMode)
{
	switch (InMode)
	{
	case ETarcopyWindowMode::Fullscreen:
		return EWindowMode::Fullscreen;
	case ETarcopyWindowMode::WindowedFullscreen:
		return EWindowMode::WindowedFullscreen;
	case ETarcopyWindowMode::Windowed:
	default:
		return EWindowMode::Windowed;
	}
}

static ETarcopyWindowMode FromWindowMode(EWindowMode::Type InMode)
{
	switch (InMode)
	{
	case EWindowMode::Fullscreen:
		return ETarcopyWindowMode::Fullscreen;
	case EWindowMode::WindowedFullscreen:
		return ETarcopyWindowMode::WindowedFullscreen;
	case EWindowMode::Windowed:
	default:
		return ETarcopyWindowMode::Windowed;
	}
}

void UTarcopySettingsSubsystem::SetResolution(const FIntPoint& Resolution, ETarcopyWindowMode WindowMode, bool bApply)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->SetScreenResolution(Resolution);
		Settings->SetFullscreenMode(ToWindowMode(WindowMode));

		if (bApply)
		{
			Settings->ApplySettings(false);
		}
	}
}

void UTarcopySettingsSubsystem::ApplyVideoSettings(bool bCheckForCommandLineOverrides)
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		Settings->ApplySettings(bCheckForCommandLineOverrides);
	}
}

TArray<FIntPoint> UTarcopySettingsSubsystem::GetSupportedResolutions() const
{
	TArray<FIntPoint> Resolutions;

	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
	}

	if (Resolutions.Num() == 0)
	{
		Resolutions.Add(GetCurrentResolution());
	}

	return Resolutions;
}

FIntPoint UTarcopySettingsSubsystem::GetCurrentResolution() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return Settings->GetScreenResolution();
	}

	return FIntPoint::ZeroValue;
}

ETarcopyWindowMode UTarcopySettingsSubsystem::GetCurrentWindowMode() const
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		return FromWindowMode(Settings->GetFullscreenMode());
	}

	return ETarcopyWindowMode::Windowed;
}

void UTarcopySettingsSubsystem::SetMasterVolume(float Volume)
{
	MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void UTarcopySettingsSubsystem::SetMusicVolume(float Volume)
{
	MusicVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void UTarcopySettingsSubsystem::SetSfxVolume(float Volume)
{
	SfxVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
	ApplySoundMix();
}

void UTarcopySettingsSubsystem::ConfigureSoundMix(USoundMix* InSoundMix, USoundClass* InMasterClass, USoundClass* InMusicClass, USoundClass* InSfxClass)
{
	MasterSoundMix = InSoundMix;
	MasterSoundClass = InMasterClass;
	MusicSoundClass = InMusicClass;
	SfxSoundClass = InSfxClass;
	ApplySoundMix();
}

float UTarcopySettingsSubsystem::GetMasterVolume() const
{
	return MasterVolume;
}

float UTarcopySettingsSubsystem::GetMusicVolume() const
{
	return MusicVolume;
}

float UTarcopySettingsSubsystem::GetSfxVolume() const
{
	return SfxVolume;
}

void UTarcopySettingsSubsystem::ApplySoundMix()
{
	if (!MasterSoundMix)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(World, MasterSoundMix);

	if (MasterSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MasterSoundClass, MasterVolume, 1.0f, 0.0f, true);
	}

	if (MusicSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, MusicSoundClass, MusicVolume, 1.0f, 0.0f, true);
	}

	if (SfxSoundClass)
	{
		UGameplayStatics::SetSoundMixClassOverride(World, MasterSoundMix, SfxSoundClass, SfxVolume, 1.0f, 0.0f, true);
	}
}
