#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "ContainerComponent.generated.h"

struct FContainerData;
class UInventoryData;

UCLASS()
class TARCOPY_API UContainerComponent : public UItemComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_SetComponent() override;

public:
	UInventoryData* GetInventoryData() const { return InventoryData; }
	const FContainerData* GetData();

private:
	void SetData();

private:
	UPROPERTY(Replicated)
	TObjectPtr<UInventoryData> InventoryData;

	const FContainerData* Data;
};
