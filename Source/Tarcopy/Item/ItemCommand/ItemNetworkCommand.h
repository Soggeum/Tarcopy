#pragma once

#include "CoreMinimal.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "ItemNetworkCommand.generated.h"

UCLASS()
class TARCOPY_API UItemNetworkCommand : public UItemCommandBase
{
	GENERATED_BODY()

protected:
	virtual void OnExecute(const FItemCommandContext& Context) override;

public:
	UPROPERTY()
	FItemNetworkContext ActionContext;
};
