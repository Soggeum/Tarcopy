#pragma once

#include "CoreMinimal.h"
#include "Item/ItemEnums.h"
#include "CraftData.generated.h"

USTRUCT(BlueprintType)
struct TARCOPY_API FCraftData : public FTableRowBase
{
	GENERATED_BODY()

public:
	//UPROPERTY(EditAnywhere, BlueprintReadOnly)
	//FName RequiredToolId;									// 도구 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> IngredientItems;						// 재료 아이템(Id, 갯수) 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, int32> GainedItems;							// 획득 아이템 (결과)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseDuration = 1.0f;
};
