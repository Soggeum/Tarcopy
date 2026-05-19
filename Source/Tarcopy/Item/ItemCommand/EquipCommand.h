#pragma once

#include "CoreMinimal.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "EquipCommand.generated.h"

enum class EBodyLocation : uint32;

UCLASS()
class TARCOPY_API UEquipCommand : public UItemCommandBase
{
	GENERATED_BODY()

protected:
	virtual void OnExecute(const FItemCommandContext& Context) override;

public:
	UPROPERTY()
	TWeakObjectPtr<class UItemInstance> TargetItem;

	UPROPERTY()
	EBodyLocation BodyLocation;
	UPROPERTY()
	uint8 bEquip : 1 = false;
};
