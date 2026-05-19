#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "MedicalComponent.generated.h"

struct FMedicalData;

UCLASS()
class TARCOPY_API UMedicalComponent : public UItemComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

protected:
	virtual void OnRep_SetComponent() override;

	virtual void OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext) override;

public:
	void RestoreHealth(AActor* InInstigator);

	const FMedicalData* GetData();

private:
	void SetData();

protected:
	const FMedicalData* Data;
};
