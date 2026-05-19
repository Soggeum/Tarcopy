#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "HoldableComponent.generated.h"

enum class EHoldableSocket : uint8;
enum class EHoldableType : uint8;

UCLASS(Abstract)
class TARCOPY_API UHoldableComponent : public UItemComponentBase
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void SetHolding(bool bInIsHolding);

protected:
	UFUNCTION()
	void OnRep_Holding();

	virtual void SetOwnerHoldingItemMesh() {}
	virtual void SetOwnerAnimPreset() {}

protected:
	void SetOwnerHoldingItemMeshAtSocket(EHoldableSocket Socket);
	void SetOwnerAnimPresetByHoldableType(EHoldableType Type);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Holding)
	uint8 bIsHolding : 1 = false;
};
