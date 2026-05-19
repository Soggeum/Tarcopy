// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CarCommand.generated.h"

UENUM(BlueprintType)
enum class ECarCommand : uint8
{
	Exit,
	SitByPassenger,
	AddFuel,
	DrainFuel,
	SitByDriver
};
