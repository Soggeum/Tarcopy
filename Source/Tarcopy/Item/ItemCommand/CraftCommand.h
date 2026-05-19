#pragma once

#include "CoreMinimal.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "CraftCommand.generated.h"

UCLASS()
class TARCOPY_API UCraftCommand : public UItemCommandBase
{
	GENERATED_BODY()
	
protected:
	virtual void OnExecute(const FItemCommandContext& Context) override;

public:
	UPROPERTY()
	FName CraftTargetId;
};
