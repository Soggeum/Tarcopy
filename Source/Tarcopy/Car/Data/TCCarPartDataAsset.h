#pragma once

#include "Engine/DataAsset.h"
#include "TCCarPartDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FCarPartStat
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;
};

UCLASS(BlueprintType)
class TARCOPY_API UTCCarPartDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<FName, FCarPartStat> PartData;
	
};
