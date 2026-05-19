#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MoodleComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TARCOPY_API UMoodleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMoodleComponent();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void RestoreHunger(float InHunger);
	void RestoreThirst(float InThirst);
	void RestoreStamina(float InStamina);

protected:
	UFUNCTION()
	void ReduceHungerByTime();
	UFUNCTION()
	void ReduceThirstByTime();

	UFUNCTION()
	void OnRep_SetHunger();
	UFUNCTION()
	void OnRep_SetThirst();
	UFUNCTION()
	void OnRep_SetStamina();

	UFUNCTION()
	void UpdateHungerUI(float CurrentValue, float MaxValue);
	UFUNCTION()
	void UpdateThirstUI(float CurrentValue, float MaxValue);
	UFUNCTION()
	void UpdateStaminaUI(float CurrentValue, float MaxValue);

	void SetHunger(float InHunger);
	void SetThirst(float InThirst);
	void SetStamina(float InStamina);

protected:
	FTimerHandle HungerTimerHandle;
	FTimerHandle ThirstTimerHandle;

	UPROPERTY()
	float HungerReduceDelay;
	UPROPERTY()
	float HungerReduceAmount;
	UPROPERTY()
	float ThirstReduceDelay;
	UPROPERTY()
	float ThirstReduceAmount;

	UPROPERTY()
	float MaxHunger;
	UPROPERTY()
	float MaxThirst;
	UPROPERTY()
	float MaxStamina;

	UPROPERTY(ReplicatedUsing = OnRep_SetHunger)
	float CurrentHunger;

	UPROPERTY(ReplicatedUsing = OnRep_SetThirst)
	float CurrentThirst;

	UPROPERTY(ReplicatedUsing = OnRep_SetStamina)
	float CurrentStamina;
};