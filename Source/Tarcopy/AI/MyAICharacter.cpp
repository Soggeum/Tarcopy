// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MyAICharacter.h"
#include "Net/UnrealNetwork.h"
#include <limits>
#include "AI/ZombieController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/MyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Common/HealthComponent.h"
#include "Components/StateTreeComponent.h"
#include "Car/TCCarBase.h"
#include "Inventory/WorldContainerComponent.h"
#include "Components/BoxComponent.h"
#include "Tarcopy.h"
#include "Components/AudioComponent.h"

// Sets default values
AMyAICharacter::AMyAICharacter() :
	AttackDamage(10),
	bIsAttack(false),
	bIsHit(false)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	AIControllerClass = AZombieController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	Movement->MaxWalkSpeed = 280.f;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	VisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisionMesh"));
	VisionMesh->SetupAttachment(RootComponent);
	VisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	VisionMesh->OnComponentBeginOverlap.AddDynamic(this, &AMyAICharacter::OnVisionMeshBeginOverlap);
	VisionMesh->OnComponentEndOverlap.AddDynamic(this, &AMyAICharacter::OnVisionMeshEndOverlap);

	StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);

	WorldContainerComponent = CreateDefaultSubobject<UWorldContainerComponent>(TEXT("WorldContainerComponent"));
	WorldContainerComponent->SetContainerType(TEXT("Zombie"));
	WorldContainerComponent->SetupAttachment(RootComponent);

	EnemyAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudioComp"));
	EnemyAudioComp->SetupAttachment(GetMesh());
	EnemyAudioComp->bAutoActivate = false;
	EnemyAudioComp->bAllowSpatialization = true;
	EnemyAudioComp->SetIsReplicated(false);
}


void AMyAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		PlayEnemySound(EnemyIdleSound);
	}

	Tags.Add(FName("InVisible"));
	SetActorHiddenInGame(true);

	if (HasAuthority())
	{
		StateTreeComponent->StartLogic();
	}
	else
	{
		StateTreeComponent->StopLogic("");
	}

	if (IsValid(HealthComponent))
	{
		HealthComponent->OnDead.AddUObject(this, &AMyAICharacter::HandleDeath);
	}
}

void AMyAICharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(GetWorld()))
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void AMyAICharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsAttack);
	DOREPLIFETIME(ThisClass, bIsHit);
}

void AMyAICharacter::OnVisionMeshBeginOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}

	AMyCharacter* Player = Cast<AMyCharacter>(OtherActor);
	ATCCarBase* Car = Cast<ATCCarBase>(OtherActor);
	
	if (IsValid(Player))
	{
		if (Player->IsPlayerControlled() == false)
		{
			return;
		}
		if (OtherComp == Player->GetCapsuleComponent())
		{
			TargetActor = Player;
			StateTreeComponent->SendStateTreeEvent(ToChase);
			PlayEnemySound(EnemyFollowingSound);
			GetWorldTimerManager().ClearTimer(EndOverlapTimer);
		}
	}
	else if (IsValid(Car))
	{
		if (Car->IsPawnControlled() == false)
		{
			return;
		}
		if (OtherComp == Car->GetMesh())
		{
			TargetActor = Car;
			StateTreeComponent->SendStateTreeEvent(ToChase);
			PlayEnemySound(EnemyFollowingSound);
			GetWorldTimerManager().ClearTimer(EndOverlapTimer);
		}
	}
}

void AMyAICharacter::OnVisionMeshEndOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (TargetActor != OtherActor)
	{
		return;
	}


	AMyCharacter* Player = Cast<AMyCharacter>(OtherActor);
	ATCCarBase* Car = Cast<ATCCarBase>(OtherActor);
	if (IsValid(Player))
	{
		if (Player->IsPlayerControlled() == false)
		{
			return;
		}
		if (OtherComp == Player->GetCapsuleComponent())
		{
			PlayEnemySound(EnemyIdleSound);
			GetWorldTimerManager().SetTimer(EndOverlapTimer, this, &AMyAICharacter::ChaseToPatrol, 1.0f, false);
		}
	}
	else if (IsValid(Car))
	{
		if (Car->IsPawnControlled() == false)
		{
			return;
		}
		if (OtherComp == Car->GetMesh())
		{
			PlayEnemySound(EnemyIdleSound);
			GetWorldTimerManager().SetTimer(EndOverlapTimer, this, &AMyAICharacter::ChaseToPatrol, 4.0f, false);
		}
	}
}

void AMyAICharacter::OnRep_bIsAttack()
{
	if (bIsAttack)
	{
		PlayAnimMontage(AM_Attack);
	}
	else
	{
		StopAnimMontage(AM_Attack);
	}
}

void AMyAICharacter::OnRep_bIsHit()
{
	if (bIsHit)
	{
		PlayAnimMontage(AM_Hit);
	}
	else
	{
		StopAnimMontage(AM_Hit);
	}
}

void AMyAICharacter::Attack(AMyAICharacter* ContextActor, AActor* DamagedActor)
{
	if (bIsAttack || bIsHit)
	{
		return;
	}

	FHitResult Hit;
	FVector StartLocation = ContextActor->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	FVector EndLocation = DamagedActor->GetActorLocation() + FVector(0.f, 0.f, FMath::FRandRange(0.f, 60.f));
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bTraceComplex = true;
	Params.bReturnPhysicalMaterial = true;

	bool bIsTargetHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		StartLocation,
		EndLocation,
		ECC_EnemyAttack,
		Params
	);

	if (bIsTargetHit)
	{
		APawn* HitActor = Cast<APawn>(Hit.GetActor());
		if (IsValid(HitActor))
		{
			PlayEnemySound(EnemyBiteSound);
			UGameplayStatics::ApplyPointDamage(HitActor,
				AttackDamage + FMath::FRandRange(-5.f, 5.f),
				EndLocation - StartLocation,
				Hit,
				GetController(),
				this,
				UDamageType::StaticClass());

			UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
			if (AnimInst)
			{
				PlayAnimMontage(AM_Attack);
				bIsAttack = true;
				/*GetCharacterMovement()->DisableMovement();*/
				FOnMontageEnded AttackMontageEnded;
				AttackMontageEnded.BindUObject(this, &AMyAICharacter::OnAttackMontageEnded);
				AnimInst->Montage_SetEndDelegate(AttackMontageEnded, AM_Attack);
			}
		}
	}
}

float AMyAICharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return Damage;

	Damage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	PlayEnemySound(EnemyHitSound);

	UAnimInstance* AnimInst = GetMesh()->GetAnimInstance();
	if (AnimInst)
	{
		PlayAnimMontage(AM_Hit);
		bIsHit = true;
		/*GetCharacterMovement()->DisableMovement();*/
		FOnMontageEnded HitMontageEnded;
		HitMontageEnded.BindUObject(this, &AMyAICharacter::OnHitMontageEnded);
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

	PatrolToChase(EventInstigator);

	return Damage;
}

void AMyAICharacter::HandleDeath()
{
	MultiRPC_HandleDeath();
	SetLifeSpan(60.f);
}
void AMyAICharacter::SetRagdolled()
{
	StateTreeComponent->StopLogic("Dead");
	GetCharacterMovement()->DisableMovement();
	USkeletalMeshComponent* SMComp = GetMesh();
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CapsuleComp->SetCollisionProfileName(TEXT("DeadEnemy"));
	SMComp->SetAllBodiesSimulatePhysics(true);
	SMComp->SetCollisionProfileName(TEXT("DeadEnemy"));
	//SMComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AMyAICharacter::MultiRPC_HandleDeath_Implementation()
{
	SetRagdolled();
	bIsDead = true;
	WorldContainerComponent->GetSenseBox()->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
}

void AMyAICharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsAttack = false;
	if (bInterrupted) return;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyAICharacter::OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsHit = false;
	if (bInterrupted) return;
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

void AMyAICharacter::PatrolToChase(AController* EventInstigator)
{
	if (HasAuthority() == false || IsValid(EventInstigator) == false)
	{
		return;
	}

	TargetActor = EventInstigator->GetPawn();
	SetActorRotation((TargetActor->GetActorLocation() - GetActorLocation()).Rotation());
	StateTreeComponent->SendStateTreeEvent(ToChase);
}

void AMyAICharacter::ChaseToPatrol()
{
	TargetActor = nullptr;
	StateTreeComponent->SendStateTreeEvent(ToPatrol);
}

void AMyAICharacter::OnRep_bIsDead()
{
	if (bIsDead)
	{
		EnemyAudioComp->Stop();
	}
}

void AMyAICharacter::PlayEnemySound_Implementation(USoundBase* NewSound)
{
	if (bIsDead) return;
	if (NewSound)
	{
		EnemyAudioComp->SetSound(NewSound);
		EnemyAudioComp->Play();
	}
}
