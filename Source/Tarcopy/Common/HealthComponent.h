#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDead)

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TARCOPY_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	float TakeDamage(float Damage, const FHitResult& HitResult);
	void RestoreHP(float InHP);

	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
	FORCEINLINE float GetMaxHP() const { return MaxHP; }

protected:
	UFUNCTION()
	void OnRep_PrintHP();

public:
	FOnDead OnDead;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBodyDamageModifierSetting> BodyDamageSetting;

	UPROPERTY(ReplicatedUsing = OnRep_PrintHP)
	float MaxHP = 100.0f;
	UPROPERTY(ReplicatedUsing = OnRep_PrintHP)
	float CurrentHP = 100.0f;
	UPROPERTY(Replicated)
	uint8 bIsDead : 1 = false;
};
