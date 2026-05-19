#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

// test
#include "Misc/Guid.h"

#include "ItemInstance.generated.h"

enum class EItemComponent : uint8;
struct FItemData;
class UItemComponentBase;
class UInventoryData;

DECLARE_MULTICAST_DELEGATE(FOnItemUpdated);

UCLASS(BlueprintType)
class TARCOPY_API UItemInstance : public UObject
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
	void SetItemId(const FName& InItemId);
	FORCEINLINE const FName& GetItemId() const { return ItemId; }

	const FItemData* GetData() const;

private:
	void SetData();

public:
	template <typename T>
	T* GetItemComponent() const
	{
		static_assert(TIsDerivedFrom<T, UItemComponentBase>::IsDerived, "T must be derived from UItemComponentBase");

		for (const auto& Component : ItemComponents)
		{
			if (T* CastedComponent = Cast<T>(Component.Get()))
			{
				return CastedComponent;
			}
		}

		return nullptr;
	}

	const TArray<TObjectPtr<UItemComponentBase>>& GetItemComponents() const;

	void SetOwnerObject(UObject* InOwnerObject);
	UObject* GetOwnerObject() const { return OwnerObject.IsValid() == true ? OwnerObject.Get() : nullptr; }
	UInventoryData* GetOwnerInventory() const { return OwnerInventory.IsValid() == true ? OwnerInventory.Get() : nullptr; }

	void SetOwnerCharacter(ACharacter* InOwnerCharacter);
	ACharacter* GetOwnerCharacter() const { return OwnerCharacter.IsValid() == true ? OwnerCharacter.Get() : nullptr; };

	bool HasAuthority() const;

	void CancelAllComponentActions();

	bool RemoveFromSource();

protected:
	UFUNCTION()
	void OnRep_SetData();
	UFUNCTION()
	void OnRep_ItemUpdated();
	UFUNCTION()
	void OnRep_SetOwnerCharacter();
	UFUNCTION()
	void OnRep_SetOwner();

	void InitComponents();

public:
	FOnItemUpdated OnItemUpdated;

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ItemUpdated)
	TArray<TObjectPtr<UItemComponentBase>> ItemComponents;

	UPROPERTY(ReplicatedUsing = OnRep_SetData)
	FName ItemId;

	UPROPERTY(ReplicatedUsing = OnRep_SetOwner)
	TWeakObjectPtr<UObject> OwnerObject;
	UPROPERTY(Replicated)
	TWeakObjectPtr<UInventoryData> OwnerInventory;
	UPROPERTY(ReplicatedUsing = OnRep_SetOwnerCharacter)
	TWeakObjectPtr<ACharacter> OwnerCharacter;					// 장착중일 때에만 캐릭터 저장

	const FItemData* Data = nullptr;

//test
public:
	UPROPERTY(Replicated)
	FGuid InstanceID;
	FGuid GetInstanceId() { return InstanceID; }
};
