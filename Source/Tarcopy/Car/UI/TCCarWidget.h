// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TCCarWidget.generated.h"

class UImage;
class UTextBlock;
class USizeBox;
class UProgressBar;

UCLASS()
class TARCOPY_API UTCCarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void UpdateSpeed(float InValue);

	UFUNCTION(BlueprintCallable)
	void UpdateRPM(float InValue);

	UFUNCTION(BlueprintCallable)
	void UpdateFuel(float InValue);

	UFUNCTION(BlueprintCallable)
	void UpdateCarDamage(float Ratio);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
	float MinAngle = -133.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
	float MaxAngle = 133.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
	float MaxSpeed = 240.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Speed")
	float MaxRPM = 6000.f;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageSpeedNeedle;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageRPMNeedle;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TextSpeed;

	FTimerHandle SpeedHandler;

	float CurrentSpeed;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageFuel;

	UPROPERTY(meta = (BindWidget))
	UImage* ImageCarSection;

	UPROPERTY()
	UMaterialInstanceDynamic* MIDFuel;


};
