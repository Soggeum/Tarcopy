#pragma once

#include "CoreMinimal.h"
#include "Item/ItemEnums.h"
#include "FirearmData.generated.h"

USTRUCT()
struct TARCOPY_API FFirearmData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBodyLocation BodyLocation = EBodyLocation::Hands;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHoldableSocket Socket = EHoldableSocket::RightHand;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EHoldableType HoldableType = EHoldableType::Pistol;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UAnimMontage> Montage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UNiagaraSystem> TrailEffect;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AmmunitionId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName MagazineId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinDamage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxDamage = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MinRange = 0.0f;												// 무기 공격이 밀치기로 전환되는 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxRange = 0.0f;												// 무기 최대 데미지가 적용되는 타점이자 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Accurancy = 0.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CritChance = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CritMultiplier = 1.5f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Knockback = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LoseCondition = 0.0f;
};
