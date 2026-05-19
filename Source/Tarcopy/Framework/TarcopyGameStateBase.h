// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TarcopyGameStateBase.generated.h"

/**
 * 
 */
UCLASS()
class TARCOPY_API ATarcopyGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void GoToTitle();
};
