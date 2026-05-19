#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/HoldableComponent.h"
#include "WeaponComponent.generated.h"

UCLASS(Abstract)
class TARCOPY_API UWeaponComponent : public UHoldableComponent
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void ExecuteAttack(const FVector& TargetLocation);
	virtual void OnExecuteAttack(const FVector& TargetLocation) PURE_VIRTUAL(UWeaponComponent::Attack, );

protected:
	void EnableOwnerMovement();

protected:
	FTimerHandle EnableMovementTimerHandle;

	UPROPERTY(Replicated)
	uint8 bIsAttacking : 1 = false;
};
