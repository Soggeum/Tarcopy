#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/WeaponComponent.h"
#include "MeleeWeaponComponent.generated.h"

struct FMeleeWeaponData;

UCLASS()
class TARCOPY_API UMeleeWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()
	
public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

	virtual void SetOwnerHoldingItemMesh() override;
	virtual void SetOwnerAnimPreset() override;

	virtual void OnExecuteAttack(const FVector& TargetLocation) override;

	virtual void CancelAction() override;

	virtual void BeginDestroy() override;

protected:
	virtual void OnRep_SetComponent() override;

public:
	const FMeleeWeaponData* GetData();

private:
	void CheckHit();
	bool CheckIsAttackableTarget(AActor* TargetActor);

	void SetData();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_PlayAttackMontage();
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_StopAttackMontage();
	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SoundAttack();

protected:
	UPROPERTY()
	TSet<AActor*> HitActors;

	FTimerHandle CheckHitTimerHandle;

	const static float CheckHitDelay;						// default: 근접 공격 애니메이션 시간 = 1초, 공격 시점 = 0.6초

	const FMeleeWeaponData* Data;
};
