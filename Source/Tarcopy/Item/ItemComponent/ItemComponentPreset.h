#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemComponentPreset.generated.h"

UCLASS()
class TARCOPY_API UItemComponentPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (DisallowedClasses = "/Script/Tarcopy.CraftComponent, /Script/Tarcopy.DefaultItemComponent"))
	TArray<TSubclassOf<class UItemComponentBase>> ItemComponentClasses;
};
