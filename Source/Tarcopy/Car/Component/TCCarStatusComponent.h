// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TCCarStatusComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API UTCCarStatusComponent : public UActorComponent
{
	GENERATED_BODY()


public:

	UTCCarStatusComponent();

	void UpdateSpeed(float NewSpeed);

	void UpdateGear(int32 NewGear);

	void UpdateRpm(float NewRPM);
	

public:
	float CurrentSpeed;

	float CurrentFuel;

	float MaxFuel;

	int32 CurrentGear;

	float CurrentRPM;

	float MaxSpeed = 240.f;
	
	float MaxRPM = 6000.f;

};
