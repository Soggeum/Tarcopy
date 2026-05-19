// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/TCChaosVehicleDummyWheel.h"


UTCChaosVehicleDummyWheel::UTCChaosVehicleDummyWheel()
{
	WheelRadius = 2.f;
	WheelWidth = 2.f;

	MaxBrakeTorque = 2.f;
	MaxHandBrakeTorque = 2.f;

	SuspensionMaxRaise = 2.f;
	SuspensionMaxDrop = 2.f;

	MaxSteerAngle = 2.f;

	FrictionForceMultiplier = 0.1f;
}
