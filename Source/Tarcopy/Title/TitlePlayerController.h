// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

class UUW_TitleLayout;
class UUserWidget;
class UUISubsystem;
class UUW_TitleScreen;

UCLASS()
class TARCOPY_API ATitlePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATitlePlayerController();

	virtual void BeginPlay() override;

	void JoinServer(const FString& InIPAddress);

private:
	UFUNCTION()
	void HandleJoinRequested(const FText& InIpPort);

	UFUNCTION()
	void HandleOptionRequested();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Title", Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> TitleWidgetInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Title", Meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> OptionsWidgetInstance;
	
	UPROPERTY()
	TObjectPtr<UUW_TitleScreen> TitleWidget;

	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

	UPROPERTY()
	TObjectPtr<UUW_TitleScreen> TitleScreenWidget;
};
