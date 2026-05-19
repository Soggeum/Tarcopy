// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TCCarActivate.generated.h"

class UVerticalBox;
class ATCCarBase;
class UCommandButtonWidget;

UCLASS()
class TARCOPY_API UTCCarActivate : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void Setup(ATCCarBase* InCar);

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* CommandList;
	
	UPROPERTY()
	ATCCarBase* Car;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCommandButtonWidget> CommandButtonClass;
	

};
