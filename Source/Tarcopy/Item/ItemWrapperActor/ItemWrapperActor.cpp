#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Components/SphereComponent.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Item/ItemComponent/ItemComponentBase.h"

AItemWrapperActor::AItemWrapperActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	LootSphere = CreateDefaultSubobject<USphereComponent>(TEXT("LootSphere"));
	LootSphere->SetupAttachment(SceneRoot);
	LootSphere->InitSphereRadius(50.f);
	LootSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	LootSphere->SetCollisionObjectType(ECC_WorldDynamic);
	LootSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	LootSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	LootSphere->SetGenerateOverlapEvents(false);

	DefaultMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultMesh"));
	DefaultMesh->SetupAttachment(SceneRoot);
	DefaultMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DefaultMesh->SetGenerateOverlapEvents(false);
}

void AItemWrapperActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItemWrapperActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentMeshAsset);
	DOREPLIFETIME(ThisClass, ItemInstance);
}

bool AItemWrapperActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	if (IsValid(ItemInstance) == true)
	{
		bWroteSomething |= ItemInstance->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	return bWroteSomething;
}

void AItemWrapperActor::SetItemInstance(UItemInstance* InItemInstance)
{
	if (HasAuthority() == false)
		return;

	if (IsValid(InItemInstance) == false)
		return;

	ItemInstance = InItemInstance;
	ItemInstance->SetOwnerObject(this);

	OnRep_SetItem();

	const FItemData* ItemData = ItemInstance->GetData();
	if (ItemData == nullptr)
		return;
	
	UStaticMesh* MeshAsset = ItemData->DefaultMesh;
	if (IsValid(MeshAsset) == false)
		return;

	CurrentMeshAsset = MeshAsset;
	OnRep_SetMesh();
}

void AItemWrapperActor::OnRep_SetMesh()
{
	if (IsValid(DefaultMesh) == true && IsValid(CurrentMeshAsset) == true)
	{
		DefaultMesh->SetStaticMesh(CurrentMeshAsset);
	}
}

void AItemWrapperActor::OnRep_SetItem()
{
	LootSphere->SetGenerateOverlapEvents(true);
	UpdateOverlaps();
}
