#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/ItemEnums.h"
#include "Item/Data/ClothData.h"
#include "EquipComponent.generated.h"

class UItemInstance;

USTRUCT(BlueprintType)
struct FDamageReduce
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UPhysicalMaterial> PhysMat;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ReduceAmount = 0.0f;								// 데미지 경감 (0.1면 받는 데미지 10퍼 감소)
};

USTRUCT(BlueprintType)
struct FItemDamageReduce
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDamageReduce> DamageReduces;
};

USTRUCT()
struct FEquippedItemInfo
{
	GENERATED_BODY()

	UPROPERTY()
	EBodyLocation Location = EBodyLocation::None;
	UPROPERTY()
	TObjectPtr<UItemInstance> Item;
};

UENUM()
enum class EUnequipType : uint8
{
	ReturnInventory,
	Drop,
	Destroy
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChangedEquippedItems);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API UEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipComponent();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

public:
	FORCEINLINE float GetWeight() const { return TotalWeight; }
	FORCEINLINE const TArray<FEquippedItemInfo>& GetEquippedItemInfos() const { return EquippedItemInfos; }
	UItemInstance* GetEquippedItem(EBodyLocation Bodylocation) const;

	void GetNeedToReplace(EBodyLocation BodyLocation, TArray<UItemInstance*>& OutItems) const;		// 해당 부위에 아이템을 장착했을 때, 장착 해제해야 하는 아이템들

public:
	UFUNCTION(Server, Reliable)
	void ServerRPC_EquipItem(EBodyLocation BodyLocation, UItemInstance* Item, bool bInstantiate = false);
	UFUNCTION(Server, Reliable)
	void ServerRPC_UnequipItem(UItemInstance* Item, EUnequipType Type = EUnequipType::ReturnInventory);

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_SetOwnerHoldingItemEmpty();

	void ExecuteAttack(const FVector& TargetLocation);
	void CancelActions();

	float GetFinalDamageTakenMultiplier(UPhysicalMaterial* PhysMat) const;

protected:
	void EquipItem(EBodyLocation BodyLocation, UItemInstance* Item, bool bInstantiate = false);
	void UnequipItem(UItemInstance* Item, EUnequipType Type = EUnequipType::ReturnInventory);

	bool RemoveItemFromInventory(UItemInstance* Item);

	void CalculateFinalDamageTakenMultiplier();

	UFUNCTION()
	void OnRep_OnChangedEquippedItems();

public:
	FOnChangedEquippedItems OnChangedEquippedItems;

protected:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_OnChangedEquippedItems)
	TArray<FEquippedItemInfo> EquippedItemInfos;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalWeight;

	UPROPERTY()
	TMap<TWeakObjectPtr<UItemInstance>, FItemDamageReduce> ItemDamageReduces;
	UPROPERTY()
	TMap<TObjectPtr<UPhysicalMaterial>, float> FinalDamageTakenMultiplier;

	// For Test
	UPROPERTY(EditAnywhere)
	FName TestEquippedItem = FName(TEXT("Axe1"));

	static const float WeightMultiplier;
};
