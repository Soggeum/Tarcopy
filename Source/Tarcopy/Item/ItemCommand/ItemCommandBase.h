#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Item/ItemNetworkContext.h"
#include "ItemCommandBase.generated.h"

USTRUCT()
struct TARCOPY_API FItemCommandContext
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<APlayerController> InstigatorController;
	UPROPERTY()
	TWeakObjectPtr<AActor> Instigator;
};

UCLASS(Abstract)
class TARCOPY_API UItemCommandBase : public UObject
{
	GENERATED_BODY()
	
public:
	UFUNCTION()
	void Execute(const FItemCommandContext& Context);

protected:
	virtual void OnExecute(const FItemCommandContext& Context) PURE_VIRTUAL(UItemCommandBase::OnExecute, );

public:
	UPROPERTY()
	FText TextDisplay;
	UPROPERTY()
	uint8 bExecutable : 1 = false;
};
