#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ClothDefensePreset.generated.h"

enum class EBodyLocation : uint32;

UCLASS()
class TARCOPY_API UClothDefensePreset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UPhysicalMaterial>> DefenseMats;
};
