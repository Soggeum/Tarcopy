// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_TitleScreen.h"

#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Title/TitlePlayerController.h"

UUW_TitleScreen::UUW_TitleScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleScreen::NativeConstruct()
{
	Super::NativeConstruct();

	StartBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnStartBtnClicked);
	OptionBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnOptionBtnClicked);
	ExitBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitBtnClicked);

	JoinBtn.Get()->OnClicked.AddDynamic(this, &ThisClass::OnJoinBtnClicked);
}

void UUW_TitleScreen::OnStartBtnClicked()
{
	IpPortInputBox->SetVisibility(ESlateVisibility::Visible);
	OnStartButtonClicked.Broadcast();
}

void UUW_TitleScreen::OnOptionBtnClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Option button clicked."));
	OnOptionButtonClicked.Broadcast();
}

void UUW_TitleScreen::OnExitBtnClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Exit button clicked. Quitting game."));
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
	OnExitButtonClicked.Broadcast();
}

void UUW_TitleScreen::OnJoinBtnClicked()
{
	FText IpPort = IpPortETxt->GetText();
	UE_LOG(LogTemp, Log, TEXT("Join button clicked with IP/Port: %s"), *IpPort.ToString());	
	OnJoinButtonClicked.Broadcast(IpPort);
}
