#pragma once

#include "CoreMinimal.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "DropCommand.generated.h"

UCLASS()
class TARCOPY_API UDropCommand : public UItemCommandBase
{
	GENERATED_BODY()

protected:
	virtual void OnExecute(const FItemCommandContext& Context) override;

public:
	UPROPERTY()
	TWeakObjectPtr<class UItemInstance> OwnerItem;
};
