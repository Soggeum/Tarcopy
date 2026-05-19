#pragma once

#include "CoreMinimal.h"
#include "ItemSoundData.generated.h"

USTRUCT()
struct TARCOPY_API FItemSoundData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<USoundBase> Sound;
};

