#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "FluidContainerComponent.generated.h"

struct FFluidContainerData;

UCLASS()
class TARCOPY_API UFluidContainerComponent : public UItemComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_SetComponent() override;

	virtual void OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext) override;

public:
	void Fill(float InAmount);
	UFUNCTION()
	void OnRep_PrintFluid();

	const FFluidContainerData* GetData();
	FORCEINLINE float GetAmount() const { return Amount; }

private:
	void SetData();

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_PrintFluid)
	FName ContainedFluidId;
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_PrintFluid)
	float Amount;

	const FFluidContainerData* Data;
};
