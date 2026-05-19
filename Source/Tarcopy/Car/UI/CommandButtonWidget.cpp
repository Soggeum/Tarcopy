// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/UI/CommandButtonWidget.h"
#include "Car/TCCarBase.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UCommandButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Label)
	{
		Label->SetText(FText::FromString(UEnum::GetValueAsString(Command)));
	}

	if (CommandButton)
	{
		CommandButton->OnClicked.AddDynamic(this, &UCommandButtonWidget::OnCommandClicked);
	}
}

void UCommandButtonWidget::OnCommandClicked()
{
	if (!Car) return;

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	APawn* Pawn = GetOwningPlayerPawn();
	if (!Pawn) return;

	Car->ExecuteCommand(Command, Pawn, PC);
}
