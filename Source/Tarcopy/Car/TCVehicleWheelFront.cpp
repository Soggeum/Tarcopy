// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/TCVehicleWheelFront.h"

UTCVehicleWheelFront::UTCVehicleWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}
