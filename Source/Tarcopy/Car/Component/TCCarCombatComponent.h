// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Car/Data/TCCarPartDataAsset.h"
#include "TCCarCombatComponent.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

USTRUCT()
struct FCarPartHP
{
	GENERATED_BODY()

	UPROPERTY()
	FName PartName = "None";

	UPROPERTY()
	float PartHP = 0.f;

	UPROPERTY()
	uint8 bIsDestroyed : 1 = false;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TARCOPY_API UTCCarCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTCCarCombatComponent();

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ApplyDamage(UBoxComponent* InBox, float Damage, const FVector& WorldPoint);

	bool IsPointInsideBox(UBoxComponent* InBox, const FVector& WorldPoint);
protected:
	
	void DestroyPart(UPrimitiveComponent* DestroyComponent);

	void DestroyWindow(UPrimitiveComponent* DestroyComponent);

	void DestroyWheel(UPrimitiveComponent* DestroyComponent);

	void DestroyMain(UPrimitiveComponent* DestroyComponent);

	void DestroyDefault(UPrimitiveComponent* DestroyComponent);

	void DisableWheelPhysics(UPrimitiveComponent* DestroyComponent);

	UFUNCTION()
	void OnVehicleHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	
	
public:	

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitBox")
	UBoxComponent* FrontBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitBox")
	UBoxComponent* BackBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitBox")
	UBoxComponent* RightBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitBox")
	UBoxComponent* LeftBox;

	UPROPERTY()
	TArray<UBoxComponent*> DamageZone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDamageImpulse;

	float LastHitTime = 0.f;

	UPROPERTY(EditAnywhere)
	UTCCarPartDataAsset* PartDataAsset;

	UPROPERTY(EditAnywhere)
	TMap<TObjectPtr<UPrimitiveComponent>, FCarPartStat> PartDataMap;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	//TMap<TObjectPtr<UPrimitiveComponent>, float> ComponentHealth;

	UPROPERTY()
	TMap<FName, TObjectPtr<UPrimitiveComponent>> ComponentName;

	UPROPERTY(Replicated)
	TArray<FCarPartHP> PartsHP;
	
	float DistanceRatio;

	//Test
	UPrimitiveComponent* GetTestMesh();

	UPrimitiveComponent* TestMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UStaticMeshComponent*> Meshes;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* HitSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* ExplosionSound;

	UPROPERTY(EditDefaultsOnly)
	USoundBase* CrashSound;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastCarPlaySound(USoundBase* NewSound);

	UFUNCTION(Client, Reliable)
	void ClientRPCRequestExit(APawn* InCar, APawn* InPawn, APlayerController* InPC);

	UPROPERTY(Replicated)
	uint8 DestroyedMain : 1 = false;
};
