// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/TarcopyGameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void ATarcopyGameStateBase::GoToTitle()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	static const FString TitleMap(TEXT("/Game/Tarcopy/Main/Maps/Title"));

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (PC->IsLocalController())
		{
			PC->ClientTravel(TitleMap, TRAVEL_Absolute);
			return;
		}
	}

	if (World->GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::OpenLevel(this, FName(*TitleMap));
	}
}

