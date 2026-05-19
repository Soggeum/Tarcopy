#include "Item/EquipComponent.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemInstance.h"
#include "Item/ItemComponent/WeaponComponent.h"
#include "GameFramework/Character.h"
#include "Item/DataTableSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Engine/ActorChannel.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Character/MyCharacter.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemComponent/ClothingComponent.h"
#include "Item/Data/ClothDefensePreset.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Item/ItemSpawnSubsystem.h"

const float UEquipComponent::WeightMultiplier = 0.3f;

UEquipComponent::UEquipComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UEquipComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false || Owner->HasAuthority() == false)
		return;

	for (uint32 SlotBit = 1; SlotBit < (uint32)EBodyLocation::MAX_BASE; SlotBit <<= 1)
	{
		EquippedItemInfos.Emplace((EBodyLocation)SlotBit, nullptr);
	}

	// 인벤토리 없어서 임시 테스트용으로 부위 아무데나 정해서 Equip에 넣고 캐릭터에서 Equip에 장착된 아이템 표시되게 해서 테스트 중
	UItemInstance* NewItem = NewObject<UItemInstance>(this);
	NewItem->SetItemId(TestEquippedItem);
	EquipItem(EBodyLocation::RightHand, NewItem, true);

	TArray<FName> Clothes = { FName(TEXT("TShirts0")), FName(TEXT("Bottoms0")), FName(TEXT("Back0")), FName(TEXT("Shoes0")), FName(TEXT("Face0")), FName(TEXT("Eyes0")), FName(TEXT("Head0")) };
	for (const auto& Cloth : Clothes)
	{
		NewItem = NewObject<UItemInstance>(this);
		NewItem->SetItemId(Cloth);
		EBodyLocation BodyLocation = NewItem->GetItemComponent<UClothingComponent>()->GetData()->BodyLocation;
		EquipItem(BodyLocation, NewItem, true);
	}
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquippedItemInfos);
}

bool UEquipComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (auto& EquippedItemInfo : EquippedItemInfos)
	{
		UItemInstance* Item = EquippedItemInfo.Item;
		if (IsValid(Item) == true)
		{
			bWroteSomething |= Item->ReplicateSubobjects(Channel, Bunch, RepFlags);
		}
	}
	return bWroteSomething;
}

UItemInstance* UEquipComponent::GetEquippedItem(EBodyLocation Bodylocation) const
{
	auto* EquippedItemPtr = EquippedItemInfos.FindByPredicate([Bodylocation](const FEquippedItemInfo ItemInfo)->bool { return ItemInfo.Location == Bodylocation; });
	return EquippedItemPtr != nullptr ? EquippedItemPtr->Item : nullptr;
}

void UEquipComponent::GetNeedToReplace(EBodyLocation BodyLocation, TArray<UItemInstance*>& OutItems) const
{
	int32 Result = 0;
	for (auto& EquippedItemInfo : EquippedItemInfos)
	{
		if (Exclusive(EquippedItemInfo.Location, BodyLocation) == true && IsValid(EquippedItemInfo.Item) == true)
		{
			OutItems.Add(EquippedItemInfo.Item);
		}
	}
}

void UEquipComponent::ServerRPC_EquipItem_Implementation(EBodyLocation BodyLocation, UItemInstance* Item, bool bInstantiate)
{
	EquipItem(BodyLocation, Item, bInstantiate);
}

void UEquipComponent::EquipItem(EBodyLocation BodyLocation, UItemInstance* Item, bool bInstantiate)
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwner());
	if (IsValid(OwnerCharacter) == false || OwnerCharacter->HasAuthority() == false)
		return;

	if (OwnerCharacter->HasAuthority() == false)
		return;

	const FItemData* ItemData = Item->GetData();
	if (ItemData == nullptr)
		return;

	// 아이템을 새로 생성해서 추가하는 것이 아니라면, 주변의 인벤토리에서 가져와야 장착 가능
	if (bInstantiate == false && RemoveItemFromInventory(Item) == false)
		return;

	Item->SetOwnerObject(this);
	Item->SetOwnerCharacter(OwnerCharacter);

	for (auto& EquippedItemInfo : EquippedItemInfos)
	{
		if (Exclusive(EquippedItemInfo.Location, BodyLocation) == true)
		{
			if (IsValid(EquippedItemInfo.Item) == true && EquippedItemInfo.Item != Item)
			{
				UnequipItem(EquippedItemInfo.Item);
			}
			EquippedItemInfo.Item = Item;
		}
	}

	TotalWeight += ItemData->Weight * WeightMultiplier;

	UClothingComponent* ClothComponent = Item->GetItemComponent<UClothingComponent>();
	if (IsValid(ClothComponent) == true)
	{
		const FClothData* ClothData = ClothComponent->GetData();
		if (IsValid(ClothData->DefensePreset) == true)
		{
			FItemDamageReduce NewItemDamageReduce;
			for (const auto& PhysMat : ClothData->DefensePreset->DefenseMats)
			{
				NewItemDamageReduce.DamageReduces.Emplace(PhysMat, ClothData->DamageReduce);
			}
			ItemDamageReduces.Add(Item, NewItemDamageReduce);
			CalculateFinalDamageTakenMultiplier();
		}
	}

	/*UKismetSystemLibrary::PrintString(GetWorld(),
		IsValid(Item->GetOwnerCharacter()) == true ?
		*Item->GetOwnerCharacter()->GetName() : TEXT("Item No Owner"));*/
}

void UEquipComponent::ServerRPC_UnequipItem_Implementation(UItemInstance* Item, EUnequipType Type)
{
	UnequipItem(Item, Type);
}

void UEquipComponent::UnequipItem(UItemInstance* Item, EUnequipType Type)
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwner());
	if (IsValid(OwnerCharacter) == false || OwnerCharacter->HasAuthority() == false)
		return;

	if (OwnerCharacter->HasAuthority() == false)
		return;

	if (IsValid(Item) == false)
		return;

	const FItemData* ItemData = Item->GetData();
	checkf(ItemData != nullptr, TEXT("EquipComponent => There's no equipped item's data"));

	bool bEquipped = false;
	for (auto& EquippedItemInfo : EquippedItemInfos)
	{
		if (EquippedItemInfo.Item != Item)
			continue;

		// 장착한 아이템이 지울 아이템이면 정리
		bEquipped = true;
		EquippedItemInfo.Item = nullptr;
	}

	// 대상 아이템을 장착하고 있지 않았다면 return
	if (bEquipped == false)
		return;

	Item->CancelAllComponentActions();
	Item->SetOwnerCharacter(nullptr);

	UClothingComponent* ClothComponent = Item->GetItemComponent<UClothingComponent>();
	if (IsValid(ClothComponent) == true)
	{
		ItemDamageReduces.Remove(Item);
		CalculateFinalDamageTakenMultiplier();
	}

	TotalWeight -= ItemData->Weight * WeightMultiplier;

	UHoldableComponent* Holdable = Item->GetItemComponent<UHoldableComponent>();
	if (IsValid(Holdable) == true)
	{
		NetMulticast_SetOwnerHoldingItemEmpty();
	}

	if (Type == EUnequipType::Destroy)
		return;

	if (Type == EUnequipType::ReturnInventory)
	{
		UPlayerInventoryComponent* InventoryComponent = GetOwner()->FindComponentByClass<UPlayerInventoryComponent>();
		if (IsValid(InventoryComponent) == true)
		{
			UInventoryData* PlayerInventory = InventoryComponent->GetPlayerInventoryData();
			if (IsValid(PlayerInventory) == true)
			{
				FIntPoint Origin;
				bool bRotated;
				bool bCanAddItem = PlayerInventory->CanAddItem(Item, Origin, bRotated);
				if (bCanAddItem == true && PlayerInventory->TryAddItem(Item, Origin, bRotated) == true)
					return;
			}
		}
	}

	UItemSpawnSubsystem* ItemSpawnSubsystem = GetWorld()->GetSubsystem<UItemSpawnSubsystem>();
	if (IsValid(ItemSpawnSubsystem) == false)
		return;

	ItemSpawnSubsystem->SpawnItemAtGround(GetOwner(), Item);
}

bool UEquipComponent::RemoveItemFromInventory(UItemInstance* Item)
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwner());
	if (IsValid(OwnerCharacter) == false || OwnerCharacter->HasAuthority() == false)
		return false;

	if (IsValid(Item) == false)
		return false;

	return Item->RemoveFromSource();
}

void UEquipComponent::CalculateFinalDamageTakenMultiplier()
{
	FinalDamageTakenMultiplier.Empty();

	for (const auto& Pair : ItemDamageReduces)
	{
		for (const auto& DamageReduce : Pair.Value.DamageReduces)
		{
			float& RefTargetDamageTaken = FinalDamageTakenMultiplier.FindOrAdd(DamageReduce.PhysMat, 1.0f);
			RefTargetDamageTaken *= (1.0f - DamageReduce.ReduceAmount);
		}
	}
}

void UEquipComponent::OnRep_OnChangedEquippedItems()
{
	if (OnChangedEquippedItems.IsBound() == true)
	{
		OnChangedEquippedItems.Broadcast();
	}
}

void UEquipComponent::NetMulticast_SetOwnerHoldingItemEmpty_Implementation()
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwner());
	if (IsValid(OwnerCharacter) == false)
		return;

	OwnerCharacter->SetHoldingItemMesh(nullptr);
	OwnerCharacter->SetAnimPreset(EHoldableType::None);
}

void UEquipComponent::ExecuteAttack(const FVector& TargetLocation)
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false || Owner->HasAuthority() == false)
		return;

	UItemInstance* ItemOnHand = GetEquippedItem(EBodyLocation::RightHand);
	if (IsValid(ItemOnHand) == false)
	{
		ItemOnHand = GetEquippedItem(EBodyLocation::LeftHand);
	}

	if (IsValid(ItemOnHand) == false)
		return;

	UWeaponComponent* WeaponComponent = ItemOnHand->GetItemComponent<UWeaponComponent>();
	if (IsValid(WeaponComponent) == false)
		return;

	WeaponComponent->ExecuteAttack(TargetLocation);
}

void UEquipComponent::CancelActions()
{
	UItemInstance* ItemOnHand = GetEquippedItem(EBodyLocation::RightHand);
	if (IsValid(ItemOnHand) == false)
	{
		ItemOnHand = GetEquippedItem(EBodyLocation::LeftHand);
	}

	if (IsValid(ItemOnHand) == false)
		return;

	ItemOnHand->CancelAllComponentActions();
}

float UEquipComponent::GetFinalDamageTakenMultiplier(UPhysicalMaterial* PhysMat) const
{
	if (IsValid(PhysMat) == false)
		return 1.0f;

	const float* PtrMultiplier = FinalDamageTakenMultiplier.Find(PhysMat);
	return PtrMultiplier != nullptr ? *PtrMultiplier : 1.0f;
}
