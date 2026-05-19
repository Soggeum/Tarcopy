// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Inventory/LootTables.h"
#include "LootRollSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class TARCOPY_API ULootRollSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	ULootRollSubsystem();

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	bool RollLoot(const FName ContainerType, TArray<FName>& OutItemIds);

private:
	const FContainerLootRow* FindContainerRow(const FName ContainerType) const;
	float GetItemWeightCached(const FName ItemId) const;

	void RebuildWeightCache();
	bool IsServerContext() const;

	UPROPERTY(EditAnywhere, Category = "Loot|Tables")
	TObjectPtr<UDataTable> ContainerLootTable = nullptr;

	UPROPERTY(EditAnywhere, Category = "Loot|Tables")
	TObjectPtr<UDataTable> ItemRarityWeightTable = nullptr;

	TMap<FName, float> ItemWeightCache;
};
