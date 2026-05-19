// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/LootRollSubsystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

static const TCHAR* LOOT_CONTEXT = TEXT("LootRollSubsystem");

ULootRollSubsystem::ULootRollSubsystem()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> ContainerDT(TEXT("/Game/Tarcopy/Main/Data/Looting/DT_ContainerLoot.DT_ContainerLoot"));

	static ConstructorHelpers::FObjectFinder<UDataTable> WeightDT(TEXT("/Game/Tarcopy/Main/Data/Looting/DT_ItemRarity.DT_ItemRarity"));

	if (ContainerDT.Succeeded())
	{
		ContainerLootTable = ContainerDT.Object;
	}

	if (WeightDT.Succeeded())
	{
		ItemRarityWeightTable = WeightDT.Object;
	}

	RebuildWeightCache();
}

void ULootRollSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RebuildWeightCache();
}

bool ULootRollSubsystem::RollLoot(const FName ContainerType, TArray<FName>& OutItemIds)
{
	OutItemIds.Reset();

	if (!IsServerContext())
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] RollLoot called on client. ContainerType=%s"), LOOT_CONTEXT, *ContainerType.ToString());
		return false;
	}

	FRandomStream Stream(FMath::Rand());
	if (!ContainerLootTable || !ItemRarityWeightTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] DataTables not set. ContainerLootTable=%s, ItemRarityWeightTable=%s"),
			LOOT_CONTEXT,
			ContainerLootTable ? *ContainerLootTable->GetName() : TEXT("NULL"),
			ItemRarityWeightTable ? *ItemRarityWeightTable->GetName() : TEXT("NULL"));
		return false;
	}

	const FContainerLootRow* ContainerRow = FindContainerRow(ContainerType);
	if (!ContainerRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Container row not found. ContainerType=%s"), LOOT_CONTEXT, *ContainerType.ToString());
		return false;
	}

	const int32 MinRoll = FMath::Max(0, ContainerRow->MinRoll);
	const int32 MaxRoll = FMath::Max(MinRoll, ContainerRow->MaxRoll);
	const int32 RollCount = (MaxRoll > MinRoll) ? Stream.RandRange(MinRoll, MaxRoll) : MinRoll;

	if (RollCount <= 0 || ContainerRow->ItemIds.Num() == 0)
	{
		return true;
	}

	for (int32 i = 0; i < RollCount; ++i)
	{
		float TotalWeight = 0.f;

		for (const FName& ItemId : ContainerRow->ItemIds)
		{
			const float W = GetItemWeightCached(ItemId);
			if (W > 0.f)
			{
				TotalWeight += W;
			}
		}

		if (TotalWeight <= 0.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("[%s] TotalWeight <= 0. ContainerType=%s"), LOOT_CONTEXT, *ContainerType.ToString());
			return true;
		}

		const float Pick = Stream.FRandRange(0.f, TotalWeight);

		float Acc = 0.f;
		FName Picked = NAME_None;

		for (const FName& ItemId : ContainerRow->ItemIds)
		{
			const float W = GetItemWeightCached(ItemId);
			if (W <= 0.f)
			{
				continue;
			}

			Acc += W;
			if (Pick <= Acc)
			{
				Picked = ItemId;
				break;
			}
		}

		if (Picked != NAME_None)
		{
			OutItemIds.Add(Picked);
		}
	}

	return true;
}

const FContainerLootRow* ULootRollSubsystem::FindContainerRow(const FName ContainerType) const
{
	if (!ContainerLootTable)
	{
		return nullptr;
	}

	if (const FContainerLootRow* ByRowName = ContainerLootTable->FindRow<FContainerLootRow>(ContainerType, LOOT_CONTEXT, /*bWarnIfMissing*/ false))
	{
		return ByRowName;
	}

	return nullptr;
}

float ULootRollSubsystem::GetItemWeightCached(const FName ItemId) const
{
	if (const float* Cached = ItemWeightCache.Find(ItemId))
	{
		return *Cached;
	}

	return 0.f;
}

void ULootRollSubsystem::RebuildWeightCache()
{
	ItemWeightCache.Reset();

	if (!ItemRarityWeightTable)
	{
		return;
	}

	static const FString ContextStr(TEXT("RebuildWeightCache"));
	TArray<FItemRarityWeightRow*> Rows;
	ItemRarityWeightTable->GetAllRows(ContextStr, Rows);

	for (const FItemRarityWeightRow* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}

		ItemWeightCache.Add(Row->ItemId, FMath::Max(0.f, Row->RarityWeight));
	}
}

bool ULootRollSubsystem::IsServerContext() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	return World->GetNetMode() != NM_Client;
}