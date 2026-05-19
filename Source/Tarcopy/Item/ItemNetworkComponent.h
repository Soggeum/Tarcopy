#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemNetworkComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API UItemNetworkComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UItemNetworkComponent();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_ExecuteItemAction(const struct FItemNetworkContext& NetworkContext);
	UFUNCTION(Server, Reliable)
	void ServerRPC_ExecuteCraft(const FName& CraftId);
};
