#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "ClothingComponent.generated.h"

struct FClothData;

UCLASS()
class TARCOPY_API UClothingComponent : public UItemComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

protected:
	virtual void OnRep_SetComponent() override;

public:
	const FClothData* GetData();

private:
	void SetData();

protected:
	/*UPROPERTY()
	uint8 bIsEquipped : 1 = false;*/

	const FClothData* Data;
};
