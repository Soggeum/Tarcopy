// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/Test/TCCarController.h"
#include "EnhancedInputSubsystems.h"
#include "Car/UI/TCCarWidget.h"
#include "Car/TCCarBase.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void ATCCarController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}




