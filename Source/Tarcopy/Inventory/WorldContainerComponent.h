// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WorldContainerComponent.generated.h"

class UInventoryData;
class UBoxComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARCOPY_API UWorldContainerComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UWorldContainerComponent();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

public:
	FText GetDisplayName() const { return DisplayName; }
	FIntPoint GetGridSize() const { return GridSize; }
	UInventoryData* GetInventoryData() const { return InventoryData; }
	UBoxComponent* GetSenseBox() { return SenseBox; }
	void SetContainerType(FName InType);
	FName GetContainerType() { return ContainerType; }

private:
	UFUNCTION()
	void OnRep_InventoryData();

	UPROPERTY(EditAnywhere, Category = "Container")
	FText DisplayName = FText::FromString(TEXT("Box"));

	UPROPERTY(EditAnywhere, Category = "Container")
	FName ContainerType = TEXT("Box");

	UPROPERTY(EditAnywhere, Category = "Container")
	FIntPoint GridSize = FIntPoint(5, 6);

	UPROPERTY(ReplicatedUsing = OnRep_InventoryData)
	TObjectPtr<UInventoryData> InventoryData;

	UPROPERTY()
	TObjectPtr<UBoxComponent> SenseBox;
};
