#pragma once

#include "CoreMinimal.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "OpenContainerCommand.generated.h"

UCLASS()
class TARCOPY_API UOpenContainerCommand : public UItemCommandBase
{
	GENERATED_BODY()

protected:
	virtual void OnExecute(const FItemCommandContext& Context) override;

public:
	UPROPERTY()
	TWeakObjectPtr<class UContainerComponent> OwnerComponent;
};
