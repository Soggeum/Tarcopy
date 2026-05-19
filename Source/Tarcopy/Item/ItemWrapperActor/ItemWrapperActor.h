#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemWrapperActor.generated.h"

class UItemInstance;

UCLASS()
class TARCOPY_API AItemWrapperActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AItemWrapperActor();

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;

public:
	void SetItemInstance(UItemInstance* InItemInstance);
	FORCEINLINE UItemInstance* GetItemInstance() const { return ItemInstance; }

protected:
	UFUNCTION()
	void OnRep_SetMesh();
	UFUNCTION()
	void OnRep_SetItem();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USceneComponent> SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class USphereComponent> LootSphere;						// 아이템 감지용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> DefaultMesh;

	UPROPERTY(ReplicatedUsing = OnRep_SetMesh)
	TObjectPtr<UStaticMesh> CurrentMeshAsset;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SetItem)
	TObjectPtr<UItemInstance> ItemInstance;
};
