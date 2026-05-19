#pragma once

#include "CoreMinimal.h"
#include "ContainerData.generated.h"

USTRUCT()
struct TARCOPY_API FContainerData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint ContainerBound = FIntPoint::ZeroValue;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WeightMultiplier = 0.0f;
};
