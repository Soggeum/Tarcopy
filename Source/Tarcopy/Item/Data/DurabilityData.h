#pragma once

#include "CoreMinimal.h"
#include "DurabilityData.generated.h"

USTRUCT()
struct TARCOPY_API FDurabilityData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxCondition = 0.0f;
	// 수리 도구나 관련 정보 추후 추가
};
