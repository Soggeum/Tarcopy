// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LootTables.generated.h"

USTRUCT(BlueprintType)
struct FItemRarityWeightRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Item")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Item", meta = (ClampMin = "0.0"))
	float RarityWeight = 1.0f;
};

USTRUCT(BlueprintType)
struct FContainerLootRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Container")
	FName ContainerType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Container")
	TArray<FName> ItemIds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Container", meta = (ClampMin = "0"))
	int32 MinRoll = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Container", meta = (ClampMin = "0"))
	int32 MaxRoll = 3;
};
