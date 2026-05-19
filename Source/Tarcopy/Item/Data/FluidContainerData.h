#pragma once

#include "CoreMinimal.h"
#include "FluidContainerData.generated.h"

USTRUCT()
struct TARCOPY_API FFluidContainerData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Capacity = 0.0f;
};
