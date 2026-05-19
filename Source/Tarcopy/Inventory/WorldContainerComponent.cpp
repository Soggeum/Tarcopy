// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/WorldContainerComponent.h"

#include "Inventory/InventoryData.h"
#include "Misc/Guid.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Item/ItemInstance.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "AI/MyAICharacter.h"
#include "Item/ItemSpawnSubsystem.h"
#include "Inventory/LootRollSubsystem.h"
#include <Kismet/GameplayStatics.h>

UWorldContainerComponent::UWorldContainerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
}

void UWorldContainerComponent::BeginPlay()
{
	Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    if (!SenseBox)
    {
        SenseBox = NewObject<UBoxComponent>(Owner, TEXT("SenseBox"));
        if (!SenseBox) return;

        SenseBox->SetupAttachment(this);
        SenseBox->SetRelativeLocation(FVector::ZeroVector);
        SenseBox->SetRelativeRotation(FRotator::ZeroRotator);
        SenseBox->SetRelativeScale3D(FVector::OneVector);

        Owner->AddInstanceComponent(SenseBox);
        SenseBox->RegisterComponent();

        SenseBox->SetBoxExtent(FVector(50.f, 50.f, 50.f));

        SenseBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        SenseBox->SetGenerateOverlapEvents(true);
        SenseBox->SetCollisionObjectType(ECC_GameTraceChannel1);

        SenseBox->SetCollisionResponseToAllChannels(ECR_Ignore);
        SenseBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);

        AMyAICharacter* AI = GetOwner<AMyAICharacter>();
        if (IsValid(AI))
        {
            SenseBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
        }

        SenseBox->UpdateOverlaps();
        GetOwner()->ForceNetUpdate();
    }

    if (!GetOwner()->HasAuthority())
    {
        return;
    }

	InventoryData = NewObject<UInventoryData>(this);
	InventoryData->Init(GridSize);

    ULootRollSubsystem* LootSub = UGameInstance::GetSubsystem<ULootRollSubsystem>(GetWorld()->GetGameInstance());
    if (!LootSub)
    {
        UE_LOG(LogTemp, Warning, TEXT("[WorldContainer] LootSub is null"));
        return;
    }

    TArray<FName> RolledItemIds;
    if (!LootSub->RollLoot(ContainerType, RolledItemIds))
    {
        UE_LOG(LogTemp, Warning, TEXT("[WorldContainer] RollLoot failed. ContainerType=%s"), *ContainerType.ToString());
        return;
    }

    for (const FName& ItemId : RolledItemIds)
    {
        if (ItemId.IsNone())
        {
            continue;
        }

        UItemInstance* NewItem = NewObject<UItemInstance>(InventoryData);
        NewItem->SetItemId(ItemId);

        bool bPlaced = false;
        for (int32 Y = 0; Y < GridSize.Y && !bPlaced; ++Y)
        {
            for (int32 X = 0; X < GridSize.X && !bPlaced; ++X)
            {
                if (InventoryData->TryAddItem(NewItem, FIntPoint(X, Y), false))
                {
                    bPlaced = true;
                }
            }
        }
    }

    GetOwner()->ForceNetUpdate();
}

void UWorldContainerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, InventoryData);
}

bool UWorldContainerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
    bool bWrote = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

    if (IsValid(InventoryData))
    {
        bWrote |= InventoryData->ReplicateSubobjects(Channel, Bunch, RepFlags);
    }
    return bWrote;
}

void UWorldContainerComponent::SetContainerType(FName InType)
{
    ContainerType = InType;
}

void UWorldContainerComponent::OnRep_InventoryData()
{
    if (InventoryData)
    {
        InventoryData->FixupAfterReplication();
    }
    if (!SenseBox)
    {
        return;
    }
    SenseBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SenseBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SenseBox->UpdateOverlaps();
}
