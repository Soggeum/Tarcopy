#pragma once

#include "CoreMinimal.h"
#include "Item/ItemEnums.h"
#include "MedicalData.generated.h"

USTRUCT()
struct TARCOPY_API FMedicalData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RestoreAmount = 0.0f;									// 체력 회복 수치
};
