#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BodyDamageModifierSetting.generated.h"

UCLASS()
class TARCOPY_API UBodyDamageModifierSetting : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<TObjectPtr<UPhysicalMaterial>, float> BodyMap;
};
