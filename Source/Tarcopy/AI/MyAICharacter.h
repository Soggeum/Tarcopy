// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StateTreeEvents.h"
#include "MyAICharacter.generated.h"

class UStaticMeshComponent;
class AMyCharacter;
class UStateTreeComponent;
struct FStateTreeEvent;
class UWorldContainerComponent;

UCLASS()
class TARCOPY_API AMyAICharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region Character Override
public:
	// Sets default values for this character's properties
	AMyAICharacter();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
#pragma endregion

#pragma region Vision

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Viewport", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VisionMesh;

	UFUNCTION()
	virtual void OnVisionMeshBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnVisionMeshEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	FTimerHandle EndOverlapTimer;
#pragma endregion

#pragma region Combat

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Equip", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UHealthComponent> HealthComponent;

	UPROPERTY(Replicated, ReplicatedUsing =  "OnRep_bIsAttack", EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsAttack;
	UFUNCTION()
	void OnRep_bIsAttack();

	UPROPERTY(Replicated, ReplicatedUsing = "OnRep_bIsHit", EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool bIsHit;
	UFUNCTION()
	void OnRep_bIsHit();

	int32 AttackDamage;

	UFUNCTION(BlueprintCallable)
	void Attack(AMyAICharacter* ContextActor, AActor* TargetActor);

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void HandleDeath();
	UFUNCTION(NetMulticast, Reliable)
	void MultiRPC_HandleDeath();

	//UPROPERTY(Replicated)
	//bool bIsRagdolled;

	void SetRagdolled();

	//UFUNCTION(NetMulticast, Reliable)
	//void PrintRag();
#pragma endregion

#pragma region Animation
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AM_Attack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AM_Hit;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);
#pragma endregion

#pragma region StateTree

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateTree")
	TObjectPtr<APawn> TargetActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateTree")
	FVector TargetLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateTree")
	FStateTreeEvent ToChase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "StateTree")
	FStateTreeEvent ToPatrol;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StateTree")
	TObjectPtr<UStateTreeComponent> StateTreeComponent;

protected:
	void PatrolToChase(AController* Instigator);
	void ChaseToPatrol();
#pragma endregion

#pragma region Inventory

protected:
	TObjectPtr<UWorldContainerComponent> WorldContainerComponent;

#pragma endregion

#pragma region Sound
	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* EnemyAudioComp;
	
	UPROPERTY(EditDefaultsOnly)
	USoundBase* EnemyIdleSound;
	UPROPERTY(EditDefaultsOnly)
	USoundBase* EnemyFollowingSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* EnemyBiteSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* EnemyHitSound;

	UFUNCTION(NetMulticast,Unreliable)
	void PlayEnemySound(USoundBase* NewSound);

	UPROPERTY(ReplicatedUsing = OnRep_bIsDead)
	uint8 bIsDead : 1 = false;

	UFUNCTION()
	void OnRep_bIsDead();
	
#pragma endregion
};
