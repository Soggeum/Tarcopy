#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnimPresetMap.generated.h"

class UAnimationPreset;
enum class EHoldableType : uint8;

UCLASS()
class TARCOPY_API UAnimPresetMap : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<EHoldableType, TObjectPtr<UAnimationPreset>> AnimPresets;
};
