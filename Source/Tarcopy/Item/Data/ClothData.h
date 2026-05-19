#pragma once

#include "CoreMinimal.h"
#include "Item/ItemEnums.h"
#include "ClothData.generated.h"

USTRUCT()
struct TARCOPY_API FClothData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EBodyLocation BodyLocation = EBodyLocation::Shirt;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UClothDefensePreset> DefensePreset;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageReduce = 0.0f;									// 데미지 경감 (0.1면 받는 데미지 10퍼 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MovementSpeed = 0.0f;									// 이동속도 저하
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackSpeed = 0.0f;									// 공격속도 저하
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float LossCondition = 0.0f;									// 데미지 받으면 잃는 내구도
};
