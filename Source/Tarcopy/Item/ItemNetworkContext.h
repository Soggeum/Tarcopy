#pragma once

#include "CoreMinimal.h"
#include "ItemNetworkContext.generated.h"

USTRUCT()
struct TARCOPY_API FItemNetworkContext
{
	GENERATED_BODY()

	UPROPERTY()
	class UItemComponentBase* TargetItemComponent = nullptr;

	UPROPERTY()
	FName ActionTag;
	
	UPROPERTY()
	TArray<float> FloatParams;
};
