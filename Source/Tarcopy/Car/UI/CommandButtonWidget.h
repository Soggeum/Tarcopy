// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Car/Data/CarCommand.h"
#include "CommandButtonWidget.generated.h"

class ATCCarBase;
class UButton;
class UTextBlock;

UCLASS()
class TARCOPY_API UCommandButtonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	ECarCommand Command;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	ATCCarBase* Car;



	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	UButton* CommandButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Label;

	UFUNCTION()
	void OnCommandClicked();


};
