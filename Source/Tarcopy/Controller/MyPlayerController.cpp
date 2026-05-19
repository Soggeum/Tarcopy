// Fill out your copyright notice in the Description page of Project Settings.


#include "Controller/MyPlayerController.h"
#include <EnhancedInputSubsystems.h>
#include "Item/Temp/UW_TempItem.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "UI/Inventory/UW_Inventory.h"
#include "UI/Inventory/UW_InventoryBorder.h"
#include "UI/UISubsystem.h"
#include "UI/Inventory/UW_NearbyPanel.h"
#include "Inventory/LootScannerComponent.h"
#include "UI/Inventory/UW_PlayerPanel.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Inventory/InventoryDragDropOp.h"
#include "Components/SizeBox.h"
#include "Car/TCCarBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Item/EquipComponent.h"
#include "UI/Moodle/UW_MoodleList.h"
#include "UI/Moodle/UW_MoodleIcon.h"


AMyPlayerController::AMyPlayerController() :
	IMC_Character(nullptr),
	IMC_Car(nullptr),
	MoveAction(nullptr)
{
	bReplicates = true;
}

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
		return;

	FInputModeGameAndUI GameAndUI;
	GameAndUI.SetHideCursorDuringCapture(false).SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(GameAndUI);
	bShowMouseCursor = true;

	if (IsValid(TempItemClass) == true)
	{
		TempItemInstance = CreateWidget<UUW_TempItem>(this, TempItemClass);
		if (IsValid(TempItemInstance) == true)
		{
			TempItemInstance->AddToViewport(0);
		}
	}

	if (auto* LP = GetLocalPlayer())
	{
		if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
		{
			if (auto* Widget = UIS->ShowUI(EUIType::Nearby))
			{
				UUW_NearbyPanel* NearbyPanel = Cast<UUW_NearbyPanel>(Widget);

				APawn* P = GetLocalPlayer()->GetPlayerController(GetWorld())->GetPawn();
				ULootScannerComponent* Scanner = P->FindComponentByClass<ULootScannerComponent>();

				NearbyPanel->BindScanner(Scanner);
			}
			if (auto* Widget = UIS->ShowUI(EUIType::Player))
			{
				UUW_PlayerPanel* PlayerPanel = Cast<UUW_PlayerPanel>(Widget);

				APawn* P = GetLocalPlayer()->GetPlayerController(GetWorld())->GetPawn();

				UPlayerInventoryComponent* InvComp = P->FindComponentByClass<UPlayerInventoryComponent>();
				PlayerPanel->BindPlayerInventory(InvComp);

				UEquipComponent* EquipmentComp = P->FindComponentByClass<UEquipComponent>();
				PlayerPanel->BindEquipComponent(EquipmentComp);
			}
			if (auto* Widget = UIS->ShowUI(EUIType::MoodleList))
			{
				MoodleUI = Cast<UUW_MoodleList>(Widget);
				MoodleUI->HungerIcon->SetRatio(1.0f);
				MoodleUI->ThirstIcon->SetRatio(1.0f);
			}
			if (auto* Widget = UIS->ShowUI(EUIType::Health))
			{
				HealthUI = Cast<UUW_MoodleIcon>(Widget);
				HealthUI->SetRatio(1.0f);
			}
			if (auto* Widget = UIS->ShowUI(EUIType::VoiceIndicator))
			{
			}
			
		}
	}
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			checkf(IMC_Character != nullptr, TEXT("IMC_Character is nullptr"));
			CurrentIMC = IMC_Character;
			Subsystem->AddMappingContext(IMC_Character, 0);
		}
	}
}

void AMyPlayerController::SetItem(UItemInstance* Item)
{
	if (IsValid(TempItemInstance) == false)
		return;

	TempItemInstance->SetItem(Item);
}

void AMyPlayerController::SetHungerTextUI(float CurrentValue, float MaxValue)
{
	if (!IsValid(MoodleUI))
	{
		return;
	}

	MoodleUI->HungerIcon->SetRatio(CurrentValue / MaxValue);
}

void AMyPlayerController::SetThirstTextUI(float CurrentValue, float MaxValue)
{
	if (!IsValid(MoodleUI))
	{
		return;
	}

	MoodleUI->ThirstIcon->SetRatio(CurrentValue / MaxValue);
}

void AMyPlayerController::SetStaminaTextUI(float CurrentValue, float MaxValue)
{
	//TempItemInstance->SetHunger(CurrentValue, MaxValue);
}

void AMyPlayerController::SetHealthUI(float CurrentValue, float MaxValue)
{
	if (!IsValid(HealthUI))
	{
		return;
	}

	HealthUI->SetRatio(CurrentValue / MaxValue);
}

void AMyPlayerController::ChangeIMC(UInputMappingContext* InIMC)
{
	if (!IsLocalPlayerController() || !InIMC) return;

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!Subsystem) return;

	Subsystem->RemoveMappingContext(CurrentIMC);

	Subsystem->AddMappingContext(InIMC, 0);
	CurrentIMC = InIMC;
}

void AMyPlayerController::ServerRPCSetDriver_Implementation(ATCCarBase* InCar, APawn* InPawn)
{
	InCar->DriverPawn = InPawn;
}

void AMyPlayerController::ServerRPCSetOwningCar_Implementation(APawn* InCar, APawn* InPawn, bool bIsDriver)
{
	if (!InCar) return;

	ATCCarBase* Car = Cast<ATCCarBase>(InCar);
	if (!Car || !InPawn) return;

	Car->AddPassenger(InPawn, bIsDriver);
}

void AMyPlayerController::ServerRPCChangePossess_Implementation(APawn* NewPawn)
{
	OnPossess(NewPawn);
}

void AMyPlayerController::ServerRPCRequestExit_Implementation(APawn* InPawn, APlayerController* InPC, APawn* InCar)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (!PlayerCharacter) return;

	ATCCarBase* Car = Cast<ATCCarBase>(InCar);

	Car->ShowCharacter(InPawn, InPC);
	
}
