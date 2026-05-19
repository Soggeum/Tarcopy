#pragma once

#include "CoreMinimal.h"
#include "FoodData.generated.h"

USTRUCT()
struct TARCOPY_API FFoodData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Hunger = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Thirst = 0.0f;
};
