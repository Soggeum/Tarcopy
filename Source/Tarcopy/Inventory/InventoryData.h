// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryData.generated.h"

class UItemInstance;

USTRUCT()
struct FItemPlacement
{
	GENERATED_BODY()

	UPROPERTY()
	FIntPoint Origin = {0, 0}; // 아이템 좌상단 좌표

	UPROPERTY()
	uint8 bRotated : 1 = false; // 회전 여부
};

USTRUCT()
struct FInventoryItemEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UItemInstance> Item = nullptr;

	UPROPERTY()
	FIntPoint Origin = FIntPoint::ZeroValue;

	UPROPERTY()
	bool bRotated = false;
};

USTRUCT()
struct FInventoryItemList : public FFastArraySerializer
{
	GENERATED_BODY()

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryItemEntry, FInventoryItemList>(Items, DeltaParams, *this);
	}

	UPROPERTY()
	TArray<FInventoryItemEntry> Items;

	UPROPERTY(NotReplicated)
	UInventoryData* Owner = nullptr;

	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
	void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
};

template<>
struct TStructOpsTypeTraits<FInventoryItemList> : public TStructOpsTypeTraitsBase2<FInventoryItemList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);

/**
 * 
 */
UCLASS()
class TARCOPY_API UInventoryData : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void Init(const FIntPoint& InGridSize);

	FGuid GetID() const { return InventoryID; }

	FIntPoint GetGridSize() const { return GridSize; }

	bool TryAddItem(UItemInstance* Item, const FIntPoint& Origin, bool bRotated, bool bOwnItem = true);

	bool CanAddItem(UItemInstance* Item, FIntPoint& OutOrigin, bool& bOutRotated, UItemInstance* IgnoreItem = nullptr);

	bool TryRelocateItem(UItemInstance* Item, UInventoryData* Dest, const FIntPoint& NewOrigin, bool bRotated);

	int32 GetItemCountByItemId(FName InItemId, TArray<UItemInstance*>& OutCandidates) const;

	bool TryConsumeItemsByItemId(FName InItemId, int32 Count);

	bool RemoveItem(UItemInstance* Item);

	FIntPoint GetItemSize(const UItemInstance* InItem, bool bRotated) const;

	bool CanPlaceItemPreview(const UItemInstance* Item, const UInventoryData* Source, const FIntPoint& NewOrigin, bool bRotated) const;

	FInventoryItemList& GetReplicatedItems() { return ReplicatedItems; }

	void FixupAfterReplication();

	void ForceRefreshNextTick();

	bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags);

	AActor* GetOwnerActor() const;

	FVector GetOwnerLocation() const;

private:
	bool CheckCanPlace(const UItemInstance* InItem, const FIntPoint& Origin, bool bRotated, const UItemInstance* IgnoreItem = nullptr) const;

	bool IsInBounds(int32 X, int32 Y) const;
	int32 ToIndex(int32 X, int32 Y) const;

	void RebuildCellsFromReplicatedItems();

	UFUNCTION()
	void OnRep_GridSize();

public:
	FOnInventoryChanged OnInventoryChanged;

private:
	FGuid InventoryID;

	friend struct FInventoryItemList;
	UPROPERTY(Replicated)
	FInventoryItemList ReplicatedItems;

	UPROPERTY(ReplicatedUsing = OnRep_GridSize)
	FIntPoint GridSize; // X = Width, Y = Height

	UPROPERTY()
	TArray<TWeakObjectPtr<UItemInstance>> Cells;
};
