// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInventoryComponent.generated.h"

class UInventoryData;
class AItemWrapperActor;
class ULootScannerComponent;
class UItemInstance;

DECLARE_MULTICAST_DELEGATE(FOnPlayerInventoryReady);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API UPlayerInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerInventoryComponent();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

public:
	UInventoryData* GetPlayerInventoryData() const { return PlayerInventoryData; }

	void HandleRelocatePostProcess(UInventoryData* SourceInventory, UItemInstance* Item);

	void RequestDropItemToWorld(UInventoryData* SourceInventory, UItemInstance* Item, bool bRotated);

	void RequestMoveItem(UInventoryData* Source, UItemInstance* Item, UInventoryData* Dest, FIntPoint NewOrigin, bool bRotated);

	void RequestLootFromWorld(AItemWrapperActor* WorldActor, UInventoryData* Dest, FIntPoint NewOrigin, bool bRotated);

private:
	void DropItemToWorld_Internal(UInventoryData* SourceInventory, UItemInstance* Item, bool bRotated);

	void MoveItem_Internal(UInventoryData* Source, UItemInstance* Item, UInventoryData* Dest, FIntPoint NewOrigin, bool bRotated);

	UFUNCTION(Server, Reliable)
	void Server_DropItemToWorld(UInventoryData* SourceInventory, UItemInstance* Item, bool bRotated);

	UFUNCTION(Server, Reliable)
	void Server_ConsumeGroundWorldItem(UItemInstance* Item);

	UFUNCTION(Server, Reliable)
	void Server_RequestMoveItem(UInventoryData* Source, UItemInstance* Item, UInventoryData* Dest, FIntPoint NewOrigin, bool bRotated);

	UFUNCTION(Server, Reliable)
	void Server_RequestLootFromWorld(AItemWrapperActor* WorldActor, UInventoryData* Dest, FIntPoint NewOrigin, bool bRotated);

	ULootScannerComponent* FindLootScanner() const;

	UFUNCTION()
	void OnRep_PlayerInventoryData();

	//UFUNCTION(Client, Reliable)
	//void Client_ForceRefreshInventoryUI();

public:
	FOnPlayerInventoryReady OnInventoryReady;

private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerInventoryData)
	TObjectPtr<UInventoryData> PlayerInventoryData;

	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	FIntPoint DefaultInventorySize = FIntPoint(5, 2);

	UPROPERTY(EditDefaultsOnly, Category = "World Drop")
	float DropForwardOffset = 80.f;

	UPROPERTY(EditDefaultsOnly, Category = "World Drop")
	float DropUpOffset = 10.f;
};
