#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ItemComponentBase.generated.h"

class UItemInstance;

DECLARE_MULTICAST_DELEGATE(FOnUpdatedItemComponent)

UCLASS(Abstract)
class TARCOPY_API UItemComponentBase : public UObject
{
	GENERATED_BODY()

public:
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

protected:
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Parms, struct FOutParmRec* OutParms, FFrame* Stack) override;

public:
	void ExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext);

	virtual void SetOwnerItem(UItemInstance* InOwnerItem);
	UItemInstance* GetOwnerItem() const;

	ACharacter* GetOwnerCharacter() const;

	class UInventoryData* GetOwnerInventory() const;

	bool HasAuthority() const;

	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) {}

	virtual void CancelAction() {}

protected:
	virtual void OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& NetworkContext) {}

	const struct FItemData* GetOwnerItemData() const;

	UFUNCTION()
	virtual void OnRep_SetComponent() {}

public:
	FOnUpdatedItemComponent OnUpdatedItemComponent;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_SetComponent)
	TWeakObjectPtr<UItemInstance> OwnerItem;
};
