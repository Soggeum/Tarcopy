#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "FoodComponent.generated.h"

struct FFoodData;

UCLASS()
class TARCOPY_API UFoodComponent : public UItemComponentBase
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
	FORCEINLINE float GetRemainAmount() { return Amount / TotalAmount; }
	const FFoodData* GetData();

private:
	void Ingest(AActor* InInstigator, int32 ConsumeAmount);
	UFUNCTION()
	void OnRep_PrintAmount();

	void SetData();

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_PrintAmount)
	int32 Amount = 4;										// default Amount = 4
	UPROPERTY(VisibleAnywhere, Replicated)
	float RemainToExpire;

	const FFoodData* Data;

	static const float TotalAmount;							// default Amount = 4
};
