#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AnimationPreset.generated.h"

class UBlendSpace1D;

UCLASS()
class TARCOPY_API UAnimationPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlendSpace1D> StandIdleWalkRunAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBlendSpace1D> AimingIdleWalkAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> FallLoopAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> LandAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimSequence> HitAnimation;
};
