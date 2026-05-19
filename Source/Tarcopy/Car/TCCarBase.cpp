//// Copyright Epic Games, Inc. All Rights Reserved.
//
#include "TCCarBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SpotLightComponent.h"
#include "Component/TCCarCombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Car/UI/TCCarWidget.h"
#include "UI/UISubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Controller/MyPlayerController.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Car/TCVehicleWheelFront.h"
#include "Car/TCVehicleWheelRear.h"
#include "Engine/DamageEvents.h"
#include "Car/UI/TCCarActivate.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SceneComponent.h"
#include "Character/MyCharacter.h"
#include "Character/Component/VisionComponent.h"
#include "Components/AudioComponent.h"


#define LOCTEXT_NAMESPACE "VehiclePawn"

ATCCarBase::ATCCarBase() :
	MaxFuel(100.f),
	MoveFactor(0.12),
	bLightOn(false),
	bCanMove(true),
	bCanRide(true),
	DriverPawn(nullptr),
	bEngineOn(false)
{
	bReplicates = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SceneComponent->SetupAttachment(GetMesh());

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetMesh());
	SpringArm->TargetArmLength = 2500.0f;
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetUsingAbsoluteRotation(false);
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetHiddenInGame(true);

	GetMesh()->SetSimulatePhysics(true);

	Light = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glass"));
	Light->SetupAttachment(GetRootComponent());

	HeadLight_R = CreateDefaultSubobject<USpotLightComponent>(TEXT("PointLight_R"));
	HeadLight_R->SetupAttachment(Light, FName("HeadLight_R"));
	HeadLight_R->SetVisibility(false);
	HeadLight_R->SetIntensity(120000.f);
	HeadLight_R->SetAttenuationRadius(4000.f);
	HeadLight_L = CreateDefaultSubobject<USpotLightComponent>(TEXT("PointLight_L"));
	HeadLight_L->SetupAttachment(Light, FName("HeadLight_L"));
	HeadLight_L->SetVisibility(false);
	HeadLight_L->SetIntensity(120000.f);
	HeadLight_L->SetAttenuationRadius(4000.f);

	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	ChaosVehicleMovement->SetIsReplicated(true);

	CombatComponent = CreateDefaultSubobject<UTCCarCombatComponent>(TEXT("CombatComponent"));
	if (CombatComponent)
	{
		for (auto& Zone : CombatComponent->DamageZone)
		{
			Zone->SetupAttachment(GetMesh());
		}
	}
	GetMesh()->SetMassOverrideInKg(NAME_None, 100000.f, true);
	GetMesh()->SetAngularDamping(15.0f);
	GetMesh()->SetLinearDamping(15.0f);
	//Test
	CurrentFuel = FMath::FRandRange(0.f, MaxFuel);

	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::Disabled;

	NetCullDistanceSquared = FMath::Square(15000.f);

	VisionComponent = CreateDefaultSubobject<UVisionComponent>(TEXT("VisionComponent"));
	VisionComponent->SetupAttachment(RootComponent);

	EngineAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioComp"));
	EngineAudioComp->SetupAttachment(GetMesh());
	EngineAudioComp->bAutoActivate = false;
	EngineAudioComp->bAllowSpatialization = true;
	EngineAudioComp->SetIsReplicated(false);
}

void ATCCarBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &ATCCarBase::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &ATCCarBase::Steering);


		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ATCCarBase::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ATCCarBase::Throttle);

		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ATCCarBase::Brake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Started, this, &ATCCarBase::StartBrake);
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ATCCarBase::StopBrake);


		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &ATCCarBase::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ATCCarBase::StopHandbrake);

		// Light
		EnhancedInputComponent->BindAction(LightAction, ETriggerEvent::Started, this, &ATCCarBase::ToggleLight);

		EnhancedInputComponent->BindAction(InterAction, ETriggerEvent::Started, this, &ATCCarBase::StartInterAction);

		EnhancedInputComponent->BindAction(WheelAction, ETriggerEvent::Started, this, &ATCCarBase::StartWheel);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATCCarBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATCCarBase::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ATCCarBase::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsLocallyControlled() && IsValid(CarWidgetInstance))
	{
		CarWidgetInstance->UpdateSpeed(ChaosVehicleMovement->GetForwardSpeed());
		CarWidgetInstance->UpdateRPM(ChaosVehicleMovement->GetEngineRotationSpeed());
	}
}

void ATCCarBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentFuel);
	DOREPLIFETIME(ThisClass, SteeringFactor);
	DOREPLIFETIME(ThisClass, ThrottleFactor);
	DOREPLIFETIME(ThisClass, bLightOn);
	DOREPLIFETIME(ThisClass, bCanMove);
	DOREPLIFETIME(ThisClass, Passengers);
	DOREPLIFETIME(ThisClass, bCanRide);
	DOREPLIFETIME(ThisClass, DriverPawn);
}

void ATCCarBase::UnPossessed()
{
	Super::UnPossessed();
}

float ATCCarBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0;

	if (!DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		return 0;
	}

	const FPointDamageEvent* PointEvent =
		static_cast<const FPointDamageEvent*>(&DamageEvent);
	if (!PointEvent) return 0;

	FVector WorldPoint = PointEvent->HitInfo.ImpactPoint;
	if (CombatComponent)
	{
		for (auto Zone : CombatComponent->DamageZone)
		{
			if (CombatComponent->IsPointInsideBox(Zone, WorldPoint))
			{
				CombatComponent->ApplyDamage(Zone, DamageAmount, WorldPoint);
			}
		}
	}

	return DamageAmount;
}

void ATCCarBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return;

	TWeakObjectPtr<ATCCarBase> WeakThis(this);


	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			GasHandler,
			FTimerDelegate::CreateLambda([WeakThis]()
				{
					if (!WeakThis.IsValid())
						return;

					float Consumption = 0.f;

					float CurrentRPM = WeakThis->ChaosVehicleMovement->GetEngineRotationSpeed();
					float ClampedRate = FMath::Clamp(CurrentRPM / 6000.f, 0.f, 1.f);
					Consumption += ClampedRate * WeakThis->MoveFactor;

					WeakThis->ServerRPCUpdateFuel(-Consumption);
				}),
			1.f,
			true,
			0.f
		);
	}

}

void ATCCarBase::OnRep_Controller()
{
	Super::OnRep_Controller();

	if (!IsLocallyControlled()) return;

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());

	OnRep_UpdateGas();

	PC->ChangeIMC(PC->IMC_Car);

}


void ATCCarBase::Steering(const FInputActionValue& Value)
{
	DoSteering(Value.Get<float>() * SteeringFactor);
}

void ATCCarBase::Throttle(const FInputActionValue& Value)
{
	DoThrottle(Value.Get<float>() * ThrottleFactor);
}

void ATCCarBase::Brake(const FInputActionValue& Value)
{
	DoBrake(Value.Get<float>() * ThrottleFactor);
}

void ATCCarBase::StartBrake(const FInputActionValue& Value)
{
	DoBrakeStart();
}

void ATCCarBase::StopBrake(const FInputActionValue& Value)
{
	DoBrakeStop();
}

void ATCCarBase::StartHandbrake(const FInputActionValue& Value)
{
	DoHandbrakeStart();
}

void ATCCarBase::StopHandbrake(const FInputActionValue& Value)
{
	DoHandbrakeStop();
}

void ATCCarBase::ToggleLight(const FInputActionValue& Value)
{
	ServerRPCDoHandLight();
}

void ATCCarBase::StartWheel(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
		return;

	const float Input = Value.Get<float>() * -200;
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Input, 600.f, 2500.f);
}

void ATCCarBase::StartInterAction(const FInputActionValue& Value)
{
	Activate(this);
}

void ATCCarBase::DoSteering(float SteeringValue)
{
	ChaosVehicleMovement->SetSteeringInput(SteeringValue);
}

void ATCCarBase::DoThrottle(float ThrottleValue)
{
	float Value = ThrottleValue;
	if (!bCanMove)
	{
		Value = 0.f;
	}

	ChaosVehicleMovement->SetThrottleInput(Value);

	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ATCCarBase::DoBrake(float BrakeValue)
{
	float Value = BrakeValue;
	if (!bCanMove)
	{
		Value = 0.f;
	}
	ChaosVehicleMovement->SetBrakeInput(Value);

	ChaosVehicleMovement->SetThrottleInput(0.0f);
}

void ATCCarBase::DoBrakeStart()
{
	BrakeLights(true);
}

void ATCCarBase::DoBrakeStop()
{
	BrakeLights(false);

	ChaosVehicleMovement->SetBrakeInput(0.0f);
}

void ATCCarBase::DoHandbrakeStart()
{
	ChaosVehicleMovement->SetHandbrakeInput(true);

	BrakeLights(true);
}

void ATCCarBase::DoHandbrakeStop()
{
	ChaosVehicleMovement->SetHandbrakeInput(false);

	BrakeLights(false);
}

void ATCCarBase::ServerRPCDoHandLight_Implementation()
{
	bLightOn = !bLightOn;
}

void ATCCarBase::OnRep_UpdateGas()
{
	if (!IsLocallyControlled()) return;
	if (!CarWidgetInstance) return;
	CarWidgetInstance->UpdateFuel(CurrentFuel);
}

void ATCCarBase::OnRep_bLightOn()
{
	if (bLightOn)
	{
		HeadLight_R->SetVisibility(true);
		HeadLight_L->SetVisibility(true);
	}
	else
	{
		HeadLight_R->SetVisibility(false);
		HeadLight_L->SetVisibility(false);
	}
}

void ATCCarBase::SitByPassenger(APawn* InPawn, APlayerController* InPC)
{

	if (!InPawn) return;
	AMyPlayerController* PC = Cast<AMyPlayerController>(InPC);
	if (!PC) return;
	if (!(InPawn->IsLocallyControlled())) return;
	Throttle(0.f);
	Brake(0.f);

	if (Cast<ATCCarBase>(InPawn))
	{
		PC->ServerRPCChangePossess(DriverPawn);
		PC->ServerRPCSetOwningCar(this, DriverPawn, true);
		if (CarWidgetInstance)
		{
			if (IsValid(UISubsystem))
			{
				UISubsystem->HideUI(EUIType::Car);
			}
		}
		return;
	}

	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (PlayerCharacter)
	{
		if (!Passengers.Contains(InPawn))
		{
			PlayerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PC->ServerRPCSetOwningCar(this, InPawn, false);
		}
	}

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(PlayerCharacter);
	if (MyCharacter)
	{
		MyCharacter->SetPlayerVisiblityInClient(false);
		MyCharacter->InActivateVisionComponent();
		VisionComponent->SetVisibility(true, true);
		VisionComponent->ActivateVisionComponent();
	}

}

void ATCCarBase::SitByDriver(APawn* InPawn, APlayerController* InPC)
{
	if (!(InPawn->IsLocallyControlled())) return;


	AMyPlayerController* PC = Cast<AMyPlayerController>(InPC);
	if (!PC) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (!PlayerCharacter) return;


	PlayerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ULocalPlayer* LP = PC->GetLocalPlayer();
	checkf(LP, TEXT("ATCCarBase::OnRep_Controller() LP"));

	UISubsystem = LP->GetSubsystem<UUISubsystem>();
	if (IsValid(UISubsystem))
	{
		CarWidgetInstance = Cast<UTCCarWidget>(UISubsystem->ShowUI(EUIType::Car));
	}

	PC->ServerRPCSetOwningCar(this, InPawn, true);
	PC->ServerRPCChangePossess(this);

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(PlayerCharacter);
	if (MyCharacter)
	{
		MyCharacter->SetPlayerVisiblityInClient(false);
		MyCharacter->InActivateVisionComponent();
		VisionComponent->SetVisibility(true, true);
		VisionComponent->ActivateVisionComponent();
	}
}

void ATCCarBase::OnRep_bEngineOn()
{
	//시동기능 생기면추가 이동예정
	if(bEngineOn)
	if (EngineAudioComp)
	{
		EngineAudioComp->Play();
	}

	TWeakObjectPtr<ATCCarBase> WeakThis(this);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SoundHandler,
			FTimerDelegate::CreateLambda([WeakThis]()
				{
					if (!WeakThis.IsValid())
						return;
					float SpeedKmh = WeakThis->ChaosVehicleMovement->GetForwardSpeed();
					SpeedKmh = FMath::Abs(SpeedKmh) * 0.036f;
					float Pitch = FMath::GetMappedRangeValueClamped(
						FVector2D(0.f, 240.f),
						FVector2D(0.5f, 1.6f),
						SpeedKmh
					);

					WeakThis->EngineAudioComp->SetPitchMultiplier(Pitch);
				}),
			0.02f,
			true,
			0.f
		);
	}
	if (!bEngineOn)
	{
		GetWorld()->GetTimerManager().ClearTimer(SoundHandler);
		EngineAudioComp->Stop();
	}
	//
}

void ATCCarBase::ExitVehicle(APawn* InPawn, APlayerController* InPC)
{
	if (!(InPawn->IsLocallyControlled())) return;
	if (!InPawn) return;
	AMyPlayerController* PC = Cast<AMyPlayerController>(InPC);
	if (!PC) return;

	APawn* Pawn = InPawn;
	if (Pawn == this)
	{
		Pawn = DriverPawn;
	}

	Throttle(0.f);
	Brake(0.f);

	if (CarWidgetInstance)
	{
		UISubsystem->HideUI(EUIType::Car);
	}

	AMyCharacter* MyCharacter = Cast<AMyCharacter>(Pawn);
	if (MyCharacter)
	{
		MyCharacter->SetPlayerVisiblityInClient(true);
		MyCharacter->ActivateVisionComponent();
		VisionComponent->SetVisibility(false, true);
		VisionComponent->InActivateVisionComponent();
	}

	PC->ServerRPCRequestExit(Pawn, InPC, this);
}

void ATCCarBase::OnRep_Passengers()
{
	for (auto& Passenger : Passengers)
	{
		ACharacter* Character = Cast<ACharacter>(Passenger);
		if (Character)
		{
			Character->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ATCCarBase::AddPassenger(APawn* InPawn, bool IsDriver)
{
	if (!HasAuthority()) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (!PlayerCharacter) return;

	PlayerCharacter->GetCharacterMovement()->DisableMovement();
	PlayerCharacter->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InPawn->AttachToComponent(
		SceneComponent,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	InPawn->SetActorRelativeLocation(FVector::ZeroVector);
	InPawn->SetActorRelativeRotation(FRotator::ZeroRotator);

	if (IsDriver)
	{
		if (!DriverPawn)
		{
			bEngineOn = true;
			DriverPawn = InPawn;
			IsDriverPawn = true;
		}
		else
		{
			IsDriverPawn = false;
			DriverPawn = nullptr;
		}
	}

	if (Passengers.Contains(InPawn))
	{
		return;
	}

	Passengers.Add(InPawn);
	if (Passengers.Num() >= 2)
	{
		bCanRide = false;
	}
	MulticastHideCharacter(InPawn);
}

void ATCCarBase::ServerRPCUpdateFuel_Implementation(float InValue)
{
	if (!HasAuthority()) return;
	CurrentFuel = FMath::Clamp(CurrentFuel + InValue, 0.f, MaxFuel);

	if (CurrentFuel <= 0 && bCanMove)
	{
		bCanMove = false;
	}
	if (CurrentFuel > 0 && !bCanMove)
	{
		bCanMove = true;
	}
}

void ATCCarBase::Activate(AActor* InInstigator)
{
	APawn* Pawn = Cast<APawn>(InInstigator);
	if (!Pawn) return;

	APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
	if (!PC) return;

	ShowInterActionUI(PC);
}

void ATCCarBase::MulticastHideCharacter_Implementation(APawn* InPawn)
{
	if (!InPawn) return;
	InPawn->SetActorHiddenInGame(true);
}

bool ATCCarBase::FindDismountLocation(APawn* InPawn, FVector& OutLocation) const
{
	ACharacter* Character = Cast<ACharacter>(InPawn);
	if (!Character) return false;

	UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
	if (!Capsule) return false;

	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	const FVector CarLocation = GetActorLocation();
	const FRotator CarRotation = GetActorRotation();

	TArray<FVector> Directions = {
	-CarRotation.RotateVector(FVector::RightVector),
	 CarRotation.RotateVector(FVector::RightVector)/*,
	-CarRotation.RotateVector(FVector::ForwardVector)*/
	};

	UWorld* World = GetWorld();
	if (!World) return false;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Character);

	for (const FVector& Dir : Directions)
	{
		const FVector Candidate = CarLocation + Dir * (CapsuleRadius + 200.f);

		const FVector SweepStart = Candidate + FVector(0, 0, CapsuleHalfHeight + 50.f);
		const FVector SweepEnd = SweepStart;

		FHitResult Hit;
		bool bBlocked = World->SweepSingleByChannel(
			Hit,
			SweepStart,
			SweepEnd,
			FQuat::Identity,
			ECC_Pawn,
			FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
			Params
		);

		if (bBlocked) continue;

		const FVector FloorStart = Candidate + FVector(0, 0, 50.f);
		const FVector FloorEnd = Candidate - FVector(0, 0, 300.f);

		FHitResult FloorHit;
		bool bHasFloor = World->LineTraceSingleByChannel(
			FloorHit,
			FloorStart,
			FloorEnd,
			ECC_Visibility,
			Params
		);

		if (!bHasFloor) continue;

		OutLocation = FloorHit.ImpactPoint + FVector(0, 0, CapsuleHalfHeight);
		return true;
	}

	return false;
}

void ATCCarBase::DisableWheel(UPrimitiveComponent* DestroyComponent)
{
	if (DestroyComponent->ComponentHasTag("Front"))
	{
		SteeringFactor -= 0.5;
		ThrottleFactor -= 0.15;
	}
	else if (DestroyComponent->ComponentHasTag("Back"))
	{
		ThrottleFactor -= 0.35;
	}
}

void ATCCarBase::ExecuteCommand(ECarCommand Command, APawn* InPawn, APlayerController* InPC)
{
	if (HasAuthority()) return;
	ULocalPlayer* LP = InPC->GetLocalPlayer();
	if (!LP) return;

	if (UI)
	{
		UI->RemoveFromParent();
	}

	switch (Command)
	{
	case ECarCommand::Exit:
		ExitVehicle(InPawn, InPC);
		break;

	case ECarCommand::AddFuel:
		ServerRPCUpdateFuel(30.f);
		break;

	case ECarCommand::SitByDriver:
		SitByDriver(InPawn, InPC);
		break;
	case ECarCommand::SitByPassenger:
		SitByPassenger(InPawn, InPC);
		break;
	}
}

TArray<ECarCommand> ATCCarBase::GetAvailableCommands()
{
	TArray<ECarCommand> Commands;

	if (IsLocallyControlled())
	{
		Commands.Add(ECarCommand::AddFuel);
	}
	if (bCanRide)
	{
		Commands.Add(ECarCommand::SitByPassenger);
	}
	if (!IsDriverPawn && bCanRide)
	{
		Commands.Add(ECarCommand::SitByDriver);
	}
	APlayerController* LocalPC = GetWorld()->GetFirstPlayerController();
	APawn* Pawn = LocalPC->GetPawn();
	if (IsLocallyControlled())
	{
		Commands.Add(ECarCommand::Exit);
	}
	else
	{
		if (Passengers.Contains(Pawn))
		{
			Commands.Add(ECarCommand::Exit);
		}
	}

	return Commands;
}

void ATCCarBase::ShowInterActionUI(APlayerController* InPC)
{
	if (!InPC) return;

	FVector2D ScreenPos = UWidgetLayoutLibrary::GetMousePositionOnViewport(InPC);

	ULocalPlayer* LP = InPC->GetLocalPlayer();
	checkf(LP, TEXT("ATCCarBase::OnRep_Controller() LP"));

	UISubsystem = LP->GetSubsystem<UUISubsystem>();

	if (IsValid(UISubsystem))
	{
		UI = UISubsystem->ShowCarCommandMenu(this, ScreenPos);
	}
	UI->SetKeyboardFocus();
}

void ATCCarBase::ShowCharacter(APawn* InPawn, APlayerController* InPC)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (!PlayerCharacter) return;
	AMyPlayerController* PC = Cast<AMyPlayerController>(InPC);
	if (!PC) return;

	FVector OutLocation = FVector::ZeroVector;
	FRotator OutRotation = GetActorRotation();
	bool bCanDismount = FindDismountLocation(InPawn, OutLocation);
	if (!bCanDismount) return;


	if (InPawn == DriverPawn)
	{
		bEngineOn = false;
		PC->ServerRPCChangePossess(DriverPawn);
		DriverPawn = nullptr;
		IsDriverPawn = false;
	}

	if (Passengers.Contains(InPawn))
	{
		Passengers.Remove(InPawn);
	}
	if (Passengers.Num() < 2)
	{
		bCanRide = true;
	}

	InPawn->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PlayerCharacter->TeleportTo(OutLocation, OutRotation);

	PlayerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	MulticastShowCharacter(InPawn, OutLocation, OutRotation);
}

void ATCCarBase::MulticastShowCharacter_Implementation(APawn* InPawn, const FVector& OutLocation, const FRotator& OutRotation)
{
	ACharacter* PlayerCharacter = Cast<ACharacter>(InPawn);
	if (!PlayerCharacter) return;

	UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent();
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	PlayerCharacter->SetActorHiddenInGame(false);

}

#undef LOCTEXT_NAMESPACE