// PlayerController for the title screen that spawns the title UI.

#include "Title/TitlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UISubsystem.h"
#include "UI/UW_TitleScreen.h"

ATitlePlayerController::ATitlePlayerController()
	: TitleWidget(nullptr)
{
	bShowMouseCursor = true;
}

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	if (IsLocalController() == false)
	{
		return;
	}

	TitleWidgetInstance = nullptr;
	TitleScreenWidget = nullptr;

	// Try UISubsystem (data-driven UI config).
	if (auto* LP = GetLocalPlayer())
	{
		if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
		{
			UISubsystem = UIS;

			if (auto* Widget = UIS->ShowUI(EUIType::Title))
			{
				TitleWidgetInstance = Widget;
				TitleScreenWidget = Cast<UUW_TitleScreen>(Widget);

				if (IsValid(TitleScreenWidget))
				{
					TitleScreenWidget->OnJoinButtonClicked.AddDynamic(this, &ThisClass::HandleJoinRequested);
					TitleScreenWidget->OnOptionButtonClicked.AddDynamic(this, &ThisClass::HandleOptionRequested);
				}
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("ATitlePlayerController: UISubsystem returned null widget for Title UI."));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ATitlePlayerController: UISubsystem not available on LocalPlayer."));
		}
	}

	if (IsValid(TitleWidgetInstance))
	{
		FInputModeUIOnly Mode;
		SetInputMode(Mode);
		bShowMouseCursor = true;
	}
}

void ATitlePlayerController::JoinServer(const FString& InIPAddress)
{
	UE_LOG(LogTemp, Warning, TEXT("JoinServer Start"));
	FString Address = InIPAddress.TrimStartAndEnd();

	if (Address.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinServer skipped: empty IP/Port input."));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Attempting to join server via OpenLevel: %s"), *Address);
	UGameplayStatics::OpenLevel(GetWorld(), FName(*Address), true);
}

void ATitlePlayerController::HandleJoinRequested(const FText& InIpPort)
{
	JoinServer(InIpPort.ToString());
}

void ATitlePlayerController::HandleOptionRequested()
{
	if (auto* Widget = UISubsystem->ShowUI(EUIType::Option))
	{
		OptionsWidgetInstance = Widget;
	}

	const bool bShow = !OptionsWidgetInstance->IsVisible();

	FInputModeUIOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = true;
}
