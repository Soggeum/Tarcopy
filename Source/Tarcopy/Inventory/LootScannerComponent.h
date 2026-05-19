// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Misc/Guid.h"
#include "LootScannerComponent.generated.h"

class USphereComponent;
class UWorldContainerComponent;
class AItemWrapperActor;
class UInventoryData;
class UItemInstance;

DECLARE_MULTICAST_DELEGATE(FOnScannedContainersChanged);
DECLARE_MULTICAST_DELEGATE(FOnScannedGroundChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API ULootScannerComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULootScannerComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	UInventoryData* GetGroundInventoryData() const { return GroundInventoryData; }

	void RebuildGroundInventory();

	bool ConsumeGroundWorldItem(UItemInstance* Item);

	AItemWrapperActor* FindWorldActorByItem(UItemInstance* Item) const;

	FOnScannedContainersChanged OnScannedContainersChanged;

	FOnScannedGroundChanged OnScannedGroundChanged;

	UPROPERTY()
	TSet<TWeakObjectPtr<UWorldContainerComponent>> OverlappedContainers;

	UPROPERTY()
	TSet<TWeakObjectPtr<AItemWrapperActor>> OverlappedContainerItems;

private:
	UFUNCTION()
	void OnContainerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnContainerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void OnGroundBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnGroundEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, Category = "Loot Scan")
	TObjectPtr<USphereComponent> ContainerSense;

	UPROPERTY(VisibleAnywhere, Category = "Loot Scan")
	TObjectPtr<USphereComponent> GroundSense;

	UPROPERTY(EditAnywhere, Category = "Loot Scan")
	float ContainerScanRadius = 800.f;

	UPROPERTY(EditAnywhere, Category = "Loot Scan")
	float GroundScanRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Loot Scan")
	FIntPoint GroundGridSize = FIntPoint(10, 10);

	UPROPERTY()
	TSet<TWeakObjectPtr<AItemWrapperActor>> OverlappedGroundItems;

	UPROPERTY()
	TObjectPtr<UInventoryData> GroundInventoryData;

	UPROPERTY()
	TMap<TWeakObjectPtr<UItemInstance>, TWeakObjectPtr<AItemWrapperActor>> ItemToWorldItem;

};
