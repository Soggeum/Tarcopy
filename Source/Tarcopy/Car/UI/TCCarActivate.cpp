// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/UI/TCCarActivate.h"
#include "Components/VerticalBox.h"
#include "Car/TCCarBase.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Car/UI/CommandButtonWidget.h"

void UTCCarActivate::NativeConstruct()
{
	Super::NativeConstruct();

	bIsFocusable = true;
}

FReply UTCCarActivate::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	RemoveFromParent();
	return FReply::Handled();
}

FReply UTCCarActivate::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	RemoveFromParent();
	return FReply::Unhandled();
}

void UTCCarActivate::Setup(ATCCarBase* InCar)
{
	Car = InCar;

	if (!Car) return;

	CommandList->ClearChildren();

	const TArray<ECarCommand> Commands = Car->GetAvailableCommands();

	for (ECarCommand Cmd : Commands)
	{
		UCommandButtonWidget* Button = CreateWidget<UCommandButtonWidget>(GetOwningPlayer(), CommandButtonClass);

		Button->Command = Cmd;
		Button->Car = Car;

		CommandList->AddChild(Button);
	}
}
