// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Controller/MyPlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Item/EquipComponent.h"
#include "Item/ItemInstance.h"
#include "Framework/DoorInteractComponent.h"
#include "Framework/DoorTagUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Character/MoodleComponent.h"
#include "AI/MyAICharacter.h"
#include "Character/ActivateInterface.h"
#include "Character/CameraObstructionFadeComponent.h"
#include "Character/CameraObstructionComponent.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemEnums.h"
#include "Misc/Guid.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Inventory/InventoryDragDropOp.h"
#include "Components/SizeBox.h"
#include "Inventory/InventoryData.h"
#include "UI/Inventory/UW_Inventory.h"
#include "Tarcopy.h"
#include "Engine/DamageEvents.h"
#include "Character/Anim/AnimPresetMap.h"
#include "Character/Anim/AnimationPreset.h"
#include "Character/Anim/PlayerAnimInstance.h"
#include "Inventory/WorldContainerComponent.h"
#include "Item/ItemComponent/ContainerComponent.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/LootScannerComponent.h"
#include "Common/HealthComponent.h"
#include "Animation/AnimMontage.h"
#include <EnhancedInputSubsystems.h>
#include "UI/UISubsystem.h"
#include "Framework/TarcopyGameStateBase.h"
#include "Character/Component/VisionComponent.h"
#include "Item/ItemNetworkComponent.h"

// Sets default values
AMyCharacter::AMyCharacter() :
	BaseWalkSpeed(400.f),
	SprintSpeedMultiplier(1.5f),
	CrouchSpeedMultiplier(0.8f),
	AimSpeedMultiplier(0.6f),
	bIsAttackMode(false)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (GetMesh())
	{
		GetMesh()->bEnableUpdateRateOptimizations = false;
		GetMesh()->bNoSkeletonUpdate = false;
		GetMesh()->bPauseAnims = false;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->SetUsingAbsoluteRotation(true);
	SpringArm->TargetArmLength = 1400.f;
	SpringArm->SetRelativeRotation(FRotator(-50.f, 45.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;

	// Create the camera component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	VisionComponent = CreateDefaultSubobject<UVisionComponent>(TEXT("VisionComponent"));
	VisionComponent->SetupAttachment(RootComponent);

	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(200.f);
	InteractionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionSphere->SetCollisionResponseToAllChannels(ECR_Overlap);
	InteractionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	InteractionSphere->OnComponentBeginOverlap.AddDynamic(this, &AMyCharacter::OnInteractionSphereBeginOverlap);
	InteractionSphere->OnComponentEndOverlap.AddDynamic(this, &AMyCharacter::OnInteractionSphereEndOverlap);

	EquipComponent = CreateDefaultSubobject<UEquipComponent>(TEXT("EquipComponent"));
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	ItemNetworkComponent = CreateDefaultSubobject<UItemNetworkComponent>(TEXT("ItemNetworkComponent"));

	Moodle = CreateDefaultSubobject<UMoodleComponent>(TEXT("Moodle"));

	CameraObstruction = CreateDefaultSubobject<UCameraObstructionComponent>(TEXT("CameraObstruction"));
	CameraObstruction->SetCamera(Camera);
	CameraObstruction->SetCapsule(GetCapsuleComponent());

	HoldingItemMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HoldingItemMeshComponent"));
	HoldingItemMeshComponent->SetupAttachment(RootComponent);
	HoldingItemMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HoldingItemMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	Inventory = CreateDefaultSubobject<UPlayerInventoryComponent>(TEXT("Inventory"));
	
	LootScanner = CreateDefaultSubobject<ULootScannerComponent>(TEXT("LootScanner"));
	LootScanner->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("InVisible"));

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDead.AddUObject(this, &AMyCharacter::HandleDeath);
	}
}

void AMyCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetWorld()))
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMyCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
	if (!PC) return;

	PC->ChangeIMC(PC->IMC_Character);
}
float AMyCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || bIsHit)
	{
		return Damage;
	}

	Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (AnimInst)
	{
		PlayAnimMontage(AM_Hit);
		bIsHit = true;
		FOnMontageEnded HitMontageEnded;
		HitMontageEnded.BindUObject(this, &AMyCharacter::OnHitMontageEnded);
		AnimInst->Montage_SetEndDelegate(HitMontageEnded, AM_Hit);
	}

	if (IsValid(HealthComponent) == true)
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)(&DamageEvent);
		if (PointDamageEvent != nullptr)
		{
			Damage = HealthComponent->TakeDamage(Damage, PointDamageEvent->HitInfo);
		}
	}

	return Damage;
}

void AMyCharacter::OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorHasDoorTagOrDoorMesh(OtherActor))
	{
		AddInteractableDoor(OtherActor);
	}
}

void AMyCharacter::OnInteractionSphereEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsValid(OtherActor))
	{
		RemoveInteractableDoor(OtherActor);
	}
}

void AMyCharacter::MoveAction(const FInputActionValue& Value)
{
	if (!IsValid(Controller))
	{
		return;
	}
	if (bIsHit)
	{
		return;
	}

	const FVector2D InMovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Camera->GetComponentRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);
}

void AMyCharacter::StartSprint(const FInputActionValue& Value)
{
	if (bIsAttackMode)
		return;

	if (GetCharacterMovement()->MaxWalkSpeed != BaseWalkSpeed * SprintSpeedMultiplier)
	{
		ServerRPC_SetSpeed(BaseWalkSpeed * SprintSpeedMultiplier);
	}
	if (!bIsCrouched)
	{
		SpringArm->TargetOffset.Z = 100.f;
	}
}

void AMyCharacter::ServerRPC_SetSpeed_Implementation(float InSpeed)
{
	if (GetCharacterMovement())
	{
		CurrentSpeed = InSpeed;
		GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
	}
}

void AMyCharacter::StopSprint(const FInputActionValue& Value)
{
	SpringArm->TargetOffset.Z = 0.f;

	if (bIsAttackMode)
		return;

	ServerRPC_SetSpeed(BaseWalkSpeed);
	if (!bIsCrouched)
	{
		SpringArm->TargetOffset.Z = 0.f;
	}
}

void AMyCharacter::OnRep_SetSpeed()
{
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
}

void AMyCharacter::StartCrouch(const FInputActionValue& Value)
{
	if (bIsAttackMode)
		return;

	ServerRPC_Crouch();
}

void AMyCharacter::ServerRPC_Crouch_Implementation()
{
	MulticastRPC_Crouch();
}

void AMyCharacter::MulticastRPC_Crouch_Implementation()
{
	if (GetCharacterMovement())
	{
		if (bIsCrouched)
		{
			GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeed;
			UnCrouch(true);
		}
		else
		{
			if (CanCrouch())
			{
				GetCharacterMovement()->MaxWalkSpeedCrouched = BaseWalkSpeed * CrouchSpeedMultiplier;
				Crouch(true);
			}
		}
	}
}

void AMyCharacter::ServerRPC_SetAiming_Implementation(bool bInIsAttackMode)
{
	bIsAttackMode = bInIsAttackMode;
}

void AMyCharacter::Wheel(const FInputActionValue& Value)
{
	if (!IsLocallyControlled())
		return;

	const float Input = Value.Get<float>() * -200;
	SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Input, 600.f, 1400.f);
}

void AMyCharacter::CanceledRightClick(const FInputActionValue& Value)
{
	bIsAttackMode = false;
	ServerRPC_SetAiming(false);

	AMyPlayerController* MyPC = GetOwner<AMyPlayerController>();
	if (!IsValid(MyPC))
		return;

	FVector WorldLocation, WorldDirection;
	if (MyPC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection) == false)
		return;

	FVector Start = Camera->GetComponentLocation();
	FVector Direction = WorldLocation - Start;
	FVector End = Start + Direction * 10000.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params) == false)
	{
		return;
	}

	if (AActor* HitActor = Hit.GetActor())
	{
		if (IActivateInterface* Activatable = Cast<IActivateInterface>(HitActor))
		{
			Activatable->Activate(this);  
			return;
		}

		TInlineComponentArray<UActorComponent*> Components(HitActor);
		for (UActorComponent* Component : Components)
		{
			if (IActivateInterface* ActivatableComponent = Cast<IActivateInterface>(Component))
			{
				ActivatableComponent->Activate(this);
				return;
			}
		}
	}
}

void AMyCharacter::TriggeredRightClick(const FInputActionValue& Value)
{
	if (bIsHit)
	{
		return;
	}

	TurnToMouse();
	SpringArm->TargetOffset.Z = 0.f;

	if (bIsAttackMode)
		return;

	bIsAttackMode = true;
	ServerRPC_SetAiming(true);
	ServerRPC_SetSpeed(BaseWalkSpeed * AimSpeedMultiplier);
}

void AMyCharacter::CompletedRightClick(const FInputActionValue& Value)
{
	SpringArm->TargetOffset.Z = 0.f;
	if (bIsAttackMode)
	{
		ServerRPC_StopTurnToMouse();
		ServerRPC_SetSpeed(BaseWalkSpeed);
		bIsAttackMode = false;
		ServerRPC_SetAiming(false);
	}
}

void AMyCharacter::LeftClick(const FInputActionValue& Value)
{
	ServerRPC_ExecuteAttack(GetAttackTargetLocation());
}

void AMyCharacter::ServerRPC_ExecuteAttack_Implementation(const FVector& TargetLocation)
{
	if (HasAuthority() == false)
		return;

	if (IsValid(EquipComponent) == false)
		return;

	EquipComponent->ExecuteAttack(TargetLocation);
}

FVector AMyCharacter::GetAttackTargetLocation() const
{
	FHitResult HitResult;
	AActor* AimTarget = nullptr;
	FName BoneName = NAME_None;
	bool bHit = GetAimTarget(AimTarget, BoneName);
	// default : 앞을 향한 충분한 먼 거리
	FVector TargetLocation = GetActorLocation() + GetActorForwardVector() * 10000.0f;
	if (IsValid(AimTarget) == true)
	{
		// 적을 조준 중이라면, 조준 중인 적의 Bone을 향해 공격
		USkeletalMeshComponent* TargetSkeletal = AimTarget->FindComponentByClass<USkeletalMeshComponent>();
		if (IsValid(TargetSkeletal) == true && TargetSkeletal->DoesSocketExist(BoneName) == true)
		{
			TargetLocation = TargetSkeletal->GetSocketLocation(BoneName);
		}
	}
	return TargetLocation;
}

void AMyCharacter::ServerRPC_TurnToMouse_Implementation(const FRotator& TargetRot)
{
	MulticastRPC_TurnToMouse(TargetRot);
}

void AMyCharacter::MulticastRPC_TurnToMouse_Implementation(const FRotator& TargetRot)
{
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetActorRotation(TargetRot);
}

void AMyCharacter::TurnToMouse()
{
	AMyPlayerController* MyPC = GetOwner<AMyPlayerController>();
	if (!IsValid(MyPC))
		return;

	FVector WorldLocation, WorldDirection;
	if (MyPC->DeprojectMousePositionToWorld(WorldLocation, WorldDirection) == false)
		return;

	FVector Start = Camera->GetComponentLocation();
	FVector Direction = WorldLocation - Start;
	FVector End = Start + Direction * 10000.f;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.bTraceComplex = true;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params) == false)
	{
		return;
	}
	FVector TargetPoint = Hit.ImpactPoint;
	FRotator TargetRot = {0.f, (TargetPoint - GetActorLocation()).Rotation().Yaw, 0.f};
	ServerRPC_TurnToMouse(TargetRot);
}

void AMyCharacter::MulticastRPC_StopTurnToMouse_Implementation()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AMyCharacter::ServerRPC_StopTurnToMouse_Implementation()
{
	MulticastRPC_StopTurnToMouse();
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this,
										  &AMyCharacter::MoveAction);
			}
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Triggered, this,
										  &AMyCharacter::StartSprint);
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this,
										  &AMyCharacter::StopSprint);
			}
			if (PlayerController->CrouchAction)
			{
				EnhancedInput->BindAction(PlayerController->CrouchAction, ETriggerEvent::Started, this,
										  &AMyCharacter::StartCrouch);
			}
			if (PlayerController->WheelAction)
			{
				EnhancedInput->BindAction(PlayerController->WheelAction, ETriggerEvent::Started, this,
										  &AMyCharacter::Wheel);
			}
			if (PlayerController->RightClickAction)
			{
				EnhancedInput->BindAction(PlayerController->RightClickAction, ETriggerEvent::Canceled, this,
										  &AMyCharacter::CanceledRightClick);
				EnhancedInput->BindAction(PlayerController->RightClickAction, ETriggerEvent::Triggered, this,
										  &AMyCharacter::TriggeredRightClick);
				EnhancedInput->BindAction(PlayerController->RightClickAction, ETriggerEvent::Completed, this,
										  &AMyCharacter::CompletedRightClick);
			}

			if (PlayerController->LeftClickAction)
			{
				EnhancedInput->BindAction(PlayerController->LeftClickAction, ETriggerEvent::Started, this,
					&AMyCharacter::LeftClick);
			}

			// 아이템 테스트용
			if (PlayerController->ItemAction)
			{
				EnhancedInput->BindAction(PlayerController->ItemAction, ETriggerEvent::Started, this,
											&AMyCharacter::SetItem);
			}
			if (PlayerController->InteractAction)
			{
				EnhancedInput->BindAction(PlayerController->InteractAction, ETriggerEvent::Started, this,
											&AMyCharacter::Interact);
			}

			// Item Rotation
			if (PlayerController->RotateAction)
			{
				EnhancedInput->BindAction(PlayerController->RotateAction, ETriggerEvent::Started, this,
					&AMyCharacter::OnRotateInventoryItem);
			}

			if (PlayerController->TabAction)
			{
				EnhancedInput->BindAction(PlayerController->TabAction, ETriggerEvent::Started, this,
					&AMyCharacter::TabAction);
			}
		}
	}
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BaseWalkSpeed);
	DOREPLIFETIME(ThisClass, SprintSpeedMultiplier);
	DOREPLIFETIME(ThisClass, CrouchSpeedMultiplier);
	DOREPLIFETIME(ThisClass, AimSpeedMultiplier);
	DOREPLIFETIME(ThisClass, CurrentSpeed);
	DOREPLIFETIME(ThisClass, bIsAttackMode);
}

void AMyCharacter::OnRep_bIsHit()
{
	if (bIsHit)
	{
		/*DisableInput(Cast<AMyPlayerController>(Controller));*/
		EquipComponent->CancelActions();
		PlayAnimMontage(AM_Hit);
	}
	else
	{
		//EnableInput(Cast<AMyPlayerController>(Controller));
		StopAnimMontage(AM_Hit);
	}
}

void AMyCharacter::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsHit = false;
	if (bInterrupted) return;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyCharacter::HandleDeath()
{
	if (HasAuthority())
	{
		GetCharacterMovement()->DisableMovement();
	}
	MultiRPC_HandleDeath();
	SetLifeSpan(300.f);
}

void AMyCharacter::ClientHandleDeath()
{
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(PC->IMC_Character);
		}

		if (auto* LP = PC->GetLocalPlayer())
		{
			if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
			{
				UIS->ResetAllUI();
			}
		}
	}

	StartFadeToBlack(5.f);
	if (IsValid(GetWorld()))
	{
		GetWorldTimerManager().SetTimer(OpenTitleLevelHandler, this, &AMyCharacter::OpenTitleLevel, 7.0f, false);
	}
}

void AMyCharacter::StartFadeToBlack(float FadeTime)
{
	APlayerCameraManager* PlayerCameraManager = GetController<APlayerController>()->PlayerCameraManager;
	if (IsValid(PlayerCameraManager))
	{
		FLinearColor FadeColor = FLinearColor::Black;
		PlayerCameraManager->StartCameraFade(0.0f, 1.0f, FadeTime, FadeColor, true, true);
	}
}

void AMyCharacter::OpenTitleLevel()
{
	if (IsValid(GetWorld()))
	{
		ATarcopyGameStateBase* GS = GetWorld()->GetGameState<ATarcopyGameStateBase>();
		if (IsValid(GS))
		{
			GS->GoToTitle();
		}
	}
}

void AMyCharacter::MultiRPC_HandleDeath_Implementation()
{
	if (IsLocallyControlled())
	{
		ClientHandleDeath();
	}
	USkeletalMeshComponent* SMComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();

	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SMComp->SetAllBodiesSimulatePhysics(true);
	CapsuleComp->SetCollisionProfileName(TEXT("Ragdoll"));
	SMComp->SetCollisionProfileName(TEXT("Ragdoll"));
}


void AMyCharacter::SetItem()
{
	if (AMyPlayerController* PlayerController = Cast<AMyPlayerController>(GetController()))
	{
		if (IsValid(EquipComponent) == false)
			return;

		const auto& EquippedItemInfos = EquipComponent->GetEquippedItemInfos();
		for (const auto& EquippedItem : EquippedItemInfos)
		{
			if (IsValid(EquippedItem.Item) == true)
			{
				PlayerController->SetItem(EquippedItem.Item);
				break;
			}
		}
	}
}

bool AMyCharacter::GetAimTarget(AActor*& OutTargetActor, FName& OutBone) const
{
	AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
	if (IsValid(PC) == false)
		return false;

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Enemy));

	FHitResult HitResult;
	if (PC->GetHitResultUnderCursorForObjects(ObjectTypes, true, HitResult))
	{
		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) == true)
		{
			OutTargetActor = HitActor;
			OutBone = HitResult.BoneName;
			return true;
		}
	}

	OutTargetActor = nullptr;
	OutBone = NAME_None;
	return false;
}

void AMyCharacter::SetHoldingItemMesh(UStaticMesh* ItemMeshAsset, const FName& SocketName)
{
	if (IsValid(HoldingItemMeshComponent) == false)
		return;

	if (IsValid(ItemMeshAsset) == false)
	{
		HoldingItemMeshComponent->SetStaticMesh(nullptr);
		return;
	}

	HoldingItemMeshComponent->SetStaticMesh(ItemMeshAsset);

	FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
	HoldingItemMeshComponent->AttachToComponent(GetMesh(), AttachRules, SocketName);
	FTransform GripPointTransform = HoldingItemMeshComponent->GetSocketTransform(TEXT("GripPoint"), RTS_Component);
	FTransform OffsetTransform = GripPointTransform.Inverse();
	HoldingItemMeshComponent->SetRelativeTransform(OffsetTransform);
}

void AMyCharacter::SetAnimPreset(EHoldableType Type)
{
	if (IsValid(AnimPresetMap) == false)
		return;

	UPlayerAnimInstance* AnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	if (IsValid(AnimInstance) == false)
		return;

	const TObjectPtr<UAnimationPreset>* Preset = AnimPresetMap->AnimPresets.Find(Type);
	if (Preset == nullptr)
		return;

	AnimInstance->SetAnimDataAsset(*Preset);
}

FVector AMyCharacter::GetAttackStartLocation() const
{
	if (IsValid(HoldingItemMeshComponent) == false)
		return GetActorLocation();
	
	return HoldingItemMeshComponent->GetSocketLocation(TEXT("Muzzle"));
}

void AMyCharacter::GetNearbyInventoryDatas(TArray<class UInventoryData*>& InventoryDatas)
{
	UPlayerInventoryComponent* InventoryComponent = FindComponentByClass<UPlayerInventoryComponent>();
	if (IsValid(InventoryComponent) == true)
	{
		UInventoryData* InventoryData = InventoryComponent->GetPlayerInventoryData();
		if (IsValid(InventoryData) == true)
		{
			InventoryDatas.Add(InventoryData);
		}
	}

	ULootScannerComponent* LootScannerComponent = FindComponentByClass<ULootScannerComponent>();
	if (IsValid(LootScannerComponent) == true)
	{
		UInventoryData* InventoryData = LootScannerComponent->GetGroundInventoryData();
		if (IsValid(InventoryData) == true)
		{
			InventoryDatas.Add(InventoryData);
		}
		for (const auto& OverlappedContainer : LootScannerComponent->OverlappedContainers)
		{
			InventoryDatas.Add(OverlappedContainer->GetInventoryData());
		}

		for (const auto& OverlappedContainerItem : LootScannerComponent->OverlappedContainerItems)
		{
			if (OverlappedContainerItem.IsValid() == false)
				continue;

			UItemInstance* ItemInstance = OverlappedContainerItem->GetItemInstance();
			if (IsValid(ItemInstance) == false)
				continue;

			UContainerComponent* ContainerComponent = ItemInstance->GetItemComponent<UContainerComponent>();
			if (IsValid(ContainerComponent) == false)
				continue;

			InventoryDatas.Add(ContainerComponent->GetInventoryData());
		}
	}
}

void AMyCharacter::AddInteractableDoor(AActor* DoorActor)
{
	if (ActorHasDoorTagOrDoorMesh(DoorActor))
	{
		OverlappingDoors.Add(DoorActor);
	}
}

void AMyCharacter::RemoveInteractableDoor(AActor* DoorActor)
{
	if (IsValid(DoorActor))
	{
		OverlappingDoors.Remove(DoorActor);
	}
}

void AMyCharacter::SetPlayerVisiblityInClient(bool bShouldVisible)
{
	if (IsValid(VisionComponent))
	{
		if (bShouldVisible)
		{
			VisionComponent->SetVisibility(true, true);
		}
		else
		{
			VisionComponent->SetVisibility(false, true);
		}
	}
}

void AMyCharacter::ActivateVisionComponent()
{
	VisionComponent->ActivateVisionComponent();
}

void AMyCharacter::InActivateVisionComponent()
{
	VisionComponent->InActivateVisionComponent();
}

void AMyCharacter::Interact(const FInputActionValue& Value)
{
	if (!IsLocallyControlled() || !IsValid(InteractionSphere))
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Interact key pressed. Overlapping door count: %d"), OverlappingDoors.Num());

	float ClosestDistSq = TNumericLimits<float>::Max();
	AActor* ClosestDoor = nullptr;

	for (const TWeakObjectPtr<AActor>& DoorPtr : OverlappingDoors)
	{
		if (!DoorPtr.IsValid())
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(DoorPtr->GetActorLocation(), GetActorLocation());
		if (DistSq < ClosestDistSq)
		{
			ClosestDistSq = DistSq;
			ClosestDoor = DoorPtr.Get();
		}
	}

	if (IsValid(ClosestDoor))
	{
		UE_LOG(LogTemp, Log, TEXT("Interact door found: %s"), *ClosestDoor->GetName());
		ServerRPC_ToggleDoor(ClosestDoor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Interact failed: no door in range."));
	}
}

void AMyCharacter::ServerRPC_ToggleDoor_Implementation(AActor* DoorActor)
{
	if (!ActorHasDoorTagOrDoorMesh(DoorActor))
	{
		UE_LOG(LogTemp, Warning, TEXT("ServerRPC_ToggleDoor aborted: invalid door actor."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AActor*> AffectedDoors;
	AffectedDoors.Reserve(2);

	static const FString DoorGroupPrefix(TEXT("DoorGroup_"));
	FName GroupTag = NAME_None;
	for (const FName& Tag : DoorActor->Tags)
	{
		if (Tag.ToString().StartsWith(DoorGroupPrefix))
		{
			GroupTag = Tag;
			break;
		}
	}

	if (GroupTag == NAME_None)
	{
		AffectedDoors.Add(DoorActor);
	}
	else
	{
		TArray<AActor*> GroupActors;
		UGameplayStatics::GetAllActorsWithTag(World, GroupTag, GroupActors);
		for (AActor* GroupDoorActor : GroupActors)
		{
			if (ActorHasDoorTagOrDoorMesh(GroupDoorActor))
			{
				AffectedDoors.AddUnique(GroupDoorActor);
			}
		}

		if (AffectedDoors.IsEmpty())
		{
			AffectedDoors.Add(DoorActor);
		}
	}

	if (IActivateInterface* Activatable = Cast<IActivateInterface>(DoorActor))
	{
		Activatable->Activate(this);
	}
	else if (UDoorInteractComponent* DoorComp = DoorActor->FindComponentByClass<UDoorInteractComponent>())
	{
		UE_LOG(LogTemp, Log, TEXT("Activating existing door component on %s"), *DoorActor->GetName());
		DoorComp->Activate(this);
	}
	else
	{
		UDoorInteractComponent* NewComp = NewObject<UDoorInteractComponent>(DoorActor);
		if (IsValid(NewComp))
		{
			NewComp->RegisterComponent();
			UE_LOG(LogTemp, Log, TEXT("Created door component and activating %s"), *DoorActor->GetName());
			NewComp->Activate(this);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create door component on %s"), *DoorActor->GetName());
			return;
		}
	}

	TArray<FTransform> DoorTransforms;
	DoorTransforms.Reserve(AffectedDoors.Num());
	TArray<bool> DoorOpenStates;
	DoorOpenStates.Reserve(AffectedDoors.Num());
	for (AActor* AffectedDoor : AffectedDoors)
	{
		if (!IsValid(AffectedDoor))
		{
			continue;
		}
		DoorTransforms.Add(AffectedDoor->GetActorTransform());

		bool bIsOpen = false;
		if (UDoorInteractComponent* DoorComp = AffectedDoor->FindComponentByClass<UDoorInteractComponent>())
		{
			bIsOpen = DoorComp->IsDoorOpen();
		}
		DoorOpenStates.Add(bIsOpen);
	}

	MulticastRPC_ApplyDoorTransforms(AffectedDoors, DoorTransforms, DoorOpenStates);
}

void AMyCharacter::MulticastRPC_ApplyDoorTransforms_Implementation(const TArray<AActor*>& DoorActors, const TArray<FTransform>& DoorTransforms, const TArray<bool>& DoorOpenStates)
{
	const int32 Count = FMath::Min3(DoorActors.Num(), DoorTransforms.Num(), DoorOpenStates.Num());
	for (int32 i = 0; i < Count; ++i)
	{
		AActor* DoorActor = DoorActors[i];
		if (!IsValid(DoorActor))
		{
			continue;
		}

		UDoorInteractComponent* DoorComp = DoorActor->FindComponentByClass<UDoorInteractComponent>();
		if (!DoorComp)
		{
			DoorComp = NewObject<UDoorInteractComponent>(DoorActor);
			if (IsValid(DoorComp))
			{
				DoorComp->RegisterComponent();
			}
		}

		if (IsValid(DoorComp))
		{
			DoorComp->ApplyDoorStateFromServer(DoorOpenStates[i]);
			continue;
		}

		if (UStaticMeshComponent* DoorMeshComp = Cast<UStaticMeshComponent>(DoorActor->GetRootComponent()))
		{
			if (DoorMeshComp->Mobility != EComponentMobility::Movable)
			{
				DoorMeshComp->SetMobility(EComponentMobility::Movable);
			}
		}

		DoorActor->SetActorTransform(DoorTransforms[i]);
	}
}

void AMyCharacter::TabAction(const FInputActionValue& Value)
{
	if (bIsVisible)
	{
		if (auto* PC = Cast<APlayerController>(GetController()))
		{
			if (auto* LP = PC->GetLocalPlayer())
			{
				if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
				{
					UIS->HideUI(EUIType::Player);
					bIsVisible = false;
				}
			}
		}
	}
	else
	{
		if (auto* PC = Cast<APlayerController>(GetController()))
		{
			if (auto* LP = PC->GetLocalPlayer())
			{
				if (auto* UIS = LP->GetSubsystem<UUISubsystem>())
				{
					UIS->ShowUI(EUIType::Player);
					bIsVisible = true;
				}
			}
		}
	}
}

void AMyCharacter::OnRotateInventoryItem()
{
	UDragDropOperation* BaseOp = UWidgetBlueprintLibrary::GetDragDroppingContent();
	UInventoryDragDropOp* Op = Cast<UInventoryDragDropOp>(BaseOp);
	if (!Op || !IsValid(Op->SourceInventory))
	{
		return;
	}

	Op->bRotated = !Op->bRotated;

	const UItemInstance* ItemPtr = Op->Item.Get();
	if (ItemPtr && IsValid(Op->DragBox) && IsValid(Op->SourceInventoryWidget))
	{
		const int32 CellPx = Op->SourceInventoryWidget->GetCellSizePx();
		const FIntPoint SizeCells = Op->SourceInventory->GetItemSize(ItemPtr, Op->bRotated);

		if (SizeCells != FIntPoint::ZeroValue)
		{
			Op->DragBox->SetWidthOverride(SizeCells.X * CellPx);
			Op->DragBox->SetHeightOverride(SizeCells.Y * CellPx);
		}
	}

	if (Op->HoveredInventoryWidget.IsValid())
	{
		Op->HoveredInventoryWidget->ForceUpdatePreviewFromOp(Op);
	}
}
