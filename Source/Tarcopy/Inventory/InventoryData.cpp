// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryData.h"

#include "Item/ItemInstance.h"
#include "Misc/Guid.h"
#include "Item/Data/ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/Actor.h"

void UInventoryData::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryData, ReplicatedItems);
	DOREPLIFETIME(UInventoryData, GridSize);
}

void UInventoryData::Init(const FIntPoint& InGridSize)
{
	GridSize = InGridSize;

	Cells.SetNum(GridSize.X * GridSize.Y);
	for (auto& Cell : Cells)
	{
		Cell = nullptr;
	}

	ReplicatedItems.Owner = this;
}

bool UInventoryData::TryAddItem(UItemInstance* Item, const FIntPoint& Origin, bool bRotated, bool bOwnItem)
{
	if (!IsValid(Item))
	{
		return false;
	}

	if (!CheckCanPlace(Item, Origin, bRotated, nullptr))
	{
		return false;
	}

	if (bOwnItem == true)
	{
		Item->SetOwnerObject(this);
	}

	FInventoryItemEntry& Entry = ReplicatedItems.Items.AddDefaulted_GetRef();
	Entry.Item = Item;
	Entry.Origin = Origin;
	Entry.bRotated = bRotated;

	ReplicatedItems.MarkItemDirty(Entry);
	ReplicatedItems.MarkArrayDirty();
	RebuildCellsFromReplicatedItems();
	return true;
}

bool UInventoryData::CanAddItem(UItemInstance* Item, FIntPoint& OutOrigin, bool& bOutRotated, UItemInstance* IgnoreItem)
{
	for (int32 Y = 0; Y < GridSize.Y; ++Y)
	{
		for (int32 X = 0; X < GridSize.X; ++X)
		{
			if (CheckCanPlace(Item, FIntPoint(X, Y), false, IgnoreItem))
			{
				OutOrigin = FIntPoint(X, Y);
				bOutRotated = false;
				return true;
			}
			if (CheckCanPlace(Item, FIntPoint(X, Y), true, IgnoreItem))
			{
				OutOrigin = FIntPoint(X, Y);
				bOutRotated = true;
				return true;
			}
		}
	}
	OutOrigin = FIntPoint(-1, -1);
	return false;
}

bool UInventoryData::TryRelocateItem(UItemInstance* Item, UInventoryData* Dest, const FIntPoint& NewOrigin, bool bRotated)
{
	if (!IsValid(Dest) || !IsValid(Item) || Item->GetData() == nullptr)
	{
		return false;
	}

	if (Dest == this)
	{
		// 같은 인벤토리에서 이동 할 경우 체크에서 본인 제외
		if (!CheckCanPlace(Item, NewOrigin, bRotated, Item))
		{
			return false;
		}

		for (FInventoryItemEntry& Entry : ReplicatedItems.Items)
		{
			if (Entry.Item == Item)
			{
				Entry.Origin = NewOrigin;
				Entry.bRotated = bRotated;
				ReplicatedItems.MarkItemDirty(Entry);
				ReplicatedItems.MarkArrayDirty();
				RebuildCellsFromReplicatedItems();
				return true;
			}
		}
		return false;
	}
	else
	{
		if (!Dest->CheckCanPlace(Item, NewOrigin, bRotated))
		{
			return false;
		}

		bool bRemoved = false;
		for (int32 i = 0; i < ReplicatedItems.Items.Num(); ++i)
		{
			if (ReplicatedItems.Items[i].Item == Item)
			{
				ReplicatedItems.Items.RemoveAt(i);
				ReplicatedItems.MarkArrayDirty();
				bRemoved = true;
				break;
			}
		}

		if (!bRemoved)
		{
			return false;
		}
		FInventoryItemEntry& NewEntry = Dest->ReplicatedItems.Items.AddDefaulted_GetRef();
		NewEntry.Item = Item;
		NewEntry.Origin = NewOrigin;
		NewEntry.bRotated = bRotated;

		Item->SetOwnerObject(Dest);

		Dest->ReplicatedItems.MarkItemDirty(NewEntry);
		Dest->ReplicatedItems.MarkArrayDirty();

		RebuildCellsFromReplicatedItems();
		Dest->RebuildCellsFromReplicatedItems();
		return true;
	}
}


int32 UInventoryData::GetItemCountByItemId(FName InItemId, TArray<UItemInstance*>& OutCandidates) const
{
	int32 Count = 0;

	for (const FInventoryItemEntry& Entry : ReplicatedItems.Items)
	{
		const UItemInstance* Item = Entry.Item;
		if (!IsValid(Item) || Item->GetData() == nullptr)
		{
			continue;
		}

		if (Item->GetData()->ItemId == InItemId)
		{
			OutCandidates.Add(Entry.Item);
			++Count;
		}
	}

	return Count;
}

bool UInventoryData::TryConsumeItemsByItemId(FName InItemId, int32 Count)
{
	if (Count <= 0)
	{
		return true;
	}

	TArray<TObjectPtr<UItemInstance>> Candidates;
	Candidates.Reserve(Count);

	// 제거 후보 수집
	for (const FInventoryItemEntry& Entry : ReplicatedItems.Items)
	{
		const UItemInstance* Item = Entry.Item;
		if (!IsValid(Item) || Item->GetData() == nullptr)
		{
			continue;
		}

		if (Item->GetData()->ItemId == InItemId)
		{
			Candidates.Add(Entry.Item);
			if (Candidates.Num() >= Count)
			{
				break;
			}
		}
	}

	// 후보가 필요 갯수 이하면 false 반환하며 제거하지 않음
	if (Candidates.Num() < Count)
	{
		return false;
	}

	// 순회하며 제거
	for (UItemInstance* Item : Candidates)
	{
		RemoveItem(Item);
	}

	return true;
}

bool UInventoryData::RemoveItem(UItemInstance* Item)
{
	if (!IsValid(Item))
	{
		return false;
	}

	for (int32 i = 0; i < ReplicatedItems.Items.Num(); ++i)
	{
		if (ReplicatedItems.Items[i].Item == Item)
		{
			ReplicatedItems.Items.RemoveAt(i);
			ReplicatedItems.MarkArrayDirty();
			RebuildCellsFromReplicatedItems();
			return true;
		}
	}
	return false;
}

FIntPoint UInventoryData::GetItemSize(const UItemInstance* InItem, bool bRotated) const
{
	if (!IsValid(InItem) || InItem->GetData() == nullptr)
	{
		return FIntPoint::ZeroValue;
	}

	FIntPoint Size = InItem->GetData()->InventoryBound;
	return bRotated ? FIntPoint(Size.Y, Size.X) : Size;
}

bool UInventoryData::CanPlaceItemPreview(const UItemInstance* Item, const UInventoryData* Source, const FIntPoint& NewOrigin, bool bRotated) const
{
	if (!IsValid(Source) || !IsValid(Item) || Item->GetData() == nullptr)
	{
		return false;
	}

	const UItemInstance* Ignore = (Source == this) ? Item : nullptr;
	return CheckCanPlace(Item, NewOrigin, bRotated, Ignore);
}

void UInventoryData::FixupAfterReplication()
{
	ReplicatedItems.Owner = this;
	RebuildCellsFromReplicatedItems();
	OnInventoryChanged.Broadcast();
}

void UInventoryData::ForceRefreshNextTick()
{
	if (UWorld* W = GetWorld())
	{
		TWeakObjectPtr<UInventoryData> WeakThis(this);
		W->GetTimerManager().SetTimerForNextTick([WeakThis]()
			{
				if (WeakThis.IsValid())
				{
					WeakThis->FixupAfterReplication();
				}
			});
	}
}

bool UInventoryData::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWrote = Channel->ReplicateSubobject(this, *Bunch, *RepFlags);

	for (const FInventoryItemEntry& Entry : ReplicatedItems.Items)
	{
		if (IsValid(Entry.Item))
		{
			bWrote |= Entry.Item->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}

	return bWrote;
}

AActor* UInventoryData::GetOwnerActor() const
{
	if (AActor* OuterActor = GetTypedOuter<AActor>())
	{
		return OuterActor;
	}

	for (UObject* OuterObj = GetOuter(); OuterObj; OuterObj = OuterObj->GetOuter())
	{
		if (AActor* A = Cast<AActor>(OuterObj))
		{
			return A;
		}
	}

	return nullptr;
}

FVector UInventoryData::GetOwnerLocation() const
{
	if (AActor* OwnerActor = GetOwnerActor())
	{
		return OwnerActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

bool UInventoryData::CheckCanPlace(const UItemInstance* InItem, const FIntPoint& Origin, bool bRotated, const UItemInstance* IgnoreItem) const
{
	const FIntPoint Size = GetItemSize(InItem, bRotated);
	if (Size == FIntPoint::ZeroValue)
	{
		return false;
	}

	for (int32 Y = 0; Y < Size.Y; ++Y)
	{
		for (int32 X = 0; X < Size.X; ++X)
		{
			const int32 CellX = Origin.X + X;
			const int32 CellY = Origin.Y + Y;

			if (!IsInBounds(CellX, CellY))
			{
				return false;
			}

			const int32 Index = ToIndex(CellX, CellY);
			if (Cells[Index].IsValid())
			{
				if (IgnoreItem && Cells[Index].Get() == IgnoreItem)
				{
					continue;
				}
				return false;
			}
		}
	}
	return true;
}

bool UInventoryData::IsInBounds(int32 X, int32 Y) const
{
	return X >= 0 && Y >= 0 && X < GridSize.X && Y < GridSize.Y;
}

int32 UInventoryData::ToIndex(int32 X, int32 Y) const
{
	return X + Y * GridSize.X;
}

void UInventoryData::RebuildCellsFromReplicatedItems()
{
	for (auto& Cell : Cells)
	{
		Cell = nullptr;
	}

	for (const FInventoryItemEntry& Entry : ReplicatedItems.Items)
	{
		if (!IsValid(Entry.Item))
		{
			continue;
		}
			
		const FIntPoint Size = GetItemSize(Entry.Item, Entry.bRotated);
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			for (int32 X = 0; X < Size.X; ++X)
			{
				const int32 CX = Entry.Origin.X + X;
				const int32 CY = Entry.Origin.Y + Y;
				if (IsInBounds(CX, CY))
				{
					Cells[ToIndex(CX, CY)] = Entry.Item;
				}
			}
		}
	}
}

void UInventoryData::OnRep_GridSize()
{
	Cells.SetNum(GridSize.X * GridSize.Y);
	for (auto& Cell : Cells)
	{ 
		Cell = nullptr; 
	}

	ReplicatedItems.Owner = this;

	RebuildCellsFromReplicatedItems();

	OnInventoryChanged.Broadcast();
}

// =========================FInventoryItemList=========================

void FInventoryItemList::PostReplicatedAdd(const TArrayView<int32>&, int32)
{
	if (Owner)
	{
		Owner->ForceRefreshNextTick();
	}

	UE_LOG(LogTemp, Warning, TEXT("[Client] PostReplicatedAdd Owner=%s (%p) IsBound=%d Items=%d"),
		*GetNameSafe(Owner), Owner,
		Owner ? (int32)Owner->OnInventoryChanged.IsBound() : -1,
		Items.Num());

}

void FInventoryItemList::PostReplicatedChange(const TArrayView<int32>&, int32)
{
	if (Owner)
	{
		Owner->ForceRefreshNextTick();
	}
}

void FInventoryItemList::PostReplicatedRemove(const TArrayView<int32>&, int32)
{
	if (Owner)
	{
		Owner->ForceRefreshNextTick();
	}
}