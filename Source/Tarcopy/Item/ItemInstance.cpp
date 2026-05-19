#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Item/DataTableSubsystem.h"
#include "Item/CraftSubsystem.h"
#include "Item/ItemComponent/CraftComponent.h"
#include "Item/ItemComponent/ItemComponentPreset.h"
#include "Item/ItemComponent/DefaultItemComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Item/ItemComponent/HoldableComponent.h"
#include "Engine/ActorChannel.h"
#include "Inventory/InventoryData.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/EquipComponent.h"

bool UItemInstance::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Channel->ReplicateSubobject(this, *Bunch, *RepFlags);
	if (OwnerObject.IsValid() == true && IsValid(Cast<AActor>(OwnerObject.Get())) == false)
	{
		bWroteSomething |= Channel->ReplicateSubobject(OwnerObject.Get(), *Bunch, *RepFlags);
	}
	if (OwnerInventory.IsValid() == true)
	{
		bWroteSomething |= Channel->ReplicateSubobject(OwnerInventory.Get(), *Bunch, *RepFlags);
	}
	for (const auto& ItemComponent : ItemComponents)
	{
		if (IsValid(ItemComponent) == false)
			continue;

		bWroteSomething |= ItemComponent->ReplicateSubobjects(Channel, Bunch, RepFlags);
	}
	return bWroteSomething;
}

bool UItemInstance::IsSupportedForNetworking() const
{
	return true;
}

void UItemInstance::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemComponents);
	DOREPLIFETIME(ThisClass, ItemId);
	DOREPLIFETIME(ThisClass, OwnerObject);
	DOREPLIFETIME(ThisClass, OwnerInventory);
	DOREPLIFETIME(ThisClass, OwnerCharacter);
	DOREPLIFETIME(ThisClass, InstanceID);
}

int32 UItemInstance::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	return IsValid(Owner) == true ? Owner->GetFunctionCallspace(Function, Stack) : FunctionCallspace::Local;
}

bool UItemInstance::CallRemoteFunction(UFunction* Function, void* Parms, FOutParmRec* OutParms, FFrame* Stack)
{
	AActor* Owner = GetTypedOuter<AActor>();
	if (UNetDriver* NetDriver = IsValid(Owner) == true ? Owner->GetNetDriver() : nullptr)
	{
		NetDriver->ProcessRemoteFunction(Owner, Function, Parms, OutParms, Stack, this);
		return true;
	}
	return false;
}

void UItemInstance::SetItemId(const FName& InItemId)
{
	ItemId = InItemId;
	OnRep_SetData();

	// 아이템 컴포넌트 생성은 서버에서만
	InitComponents();
}

const FItemData* UItemInstance::GetData() const
{
	if (Data == nullptr)
	{
		const_cast<UItemInstance*>(this)->SetData();
	}
	return Data;
}

void UItemInstance::SetData()
{
	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	const UDataTable* ItemTable = DataTableSubsystem->GetTable(EDataTableType::ItemTable);
	if (IsValid(ItemTable) == false)
		return;

	Data = ItemTable->FindRow<FItemData>(ItemId, FString(""));
}

void UItemInstance::OnRep_SetData()
{
	SetData();
	OnRep_ItemUpdated();
}

void UItemInstance::OnRep_ItemUpdated()
{
	if (OnItemUpdated.IsBound() == true)
	{
		OnItemUpdated.Broadcast();
	}
}

void UItemInstance::OnRep_SetOwnerCharacter()
{
	if (HasAuthority() == true)
	{
		UHoldableComponent* HoldableComponent = GetItemComponent<UHoldableComponent>();
		if (IsValid(HoldableComponent) == true)
		{
			HoldableComponent->SetHolding(OwnerCharacter.IsValid());
		}
	}

	OnRep_ItemUpdated();
}

void UItemInstance::OnRep_SetOwner()
{
	if (OwnerObject.IsValid() == false)
		return;

	Rename(nullptr, OwnerObject.Get());
}

void UItemInstance::InitComponents()
{
	if (Data == nullptr)
		return;

	// 당장은 의미 없음
	ItemComponents.Empty();

	if (IsValid(Data->ItemComponentPreset) == true)
	{
		for (const auto& Component : Data->ItemComponentPreset->ItemComponentClasses)
		{
			if (IsValid(Component) == false)
				continue;

			UItemComponentBase* NewItemComponent = NewObject<UItemComponentBase>(this, Component);
			NewItemComponent->SetOwnerItem(this);
			ItemComponents.Add(NewItemComponent);
		}
	}

	// CraftTable에서 ItemId가 재료로 사용되면 CraftComponent 자동 생성
	UCraftSubsystem* CraftSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UCraftSubsystem>();
	if (IsValid(CraftSubsystem) == true)
	{
		if (CraftSubsystem->GetCraftRecipe(Data->ItemId) != nullptr)
		{
			UCraftComponent* CraftComponent = NewObject<UCraftComponent>(this);
			CraftComponent->SetOwnerItem(this);
			ItemComponents.Add(CraftComponent);
		}
	}

	UItemComponentBase* NewItemComponent = NewObject<UDefaultItemComponent>(this);
	NewItemComponent->SetOwnerItem(this);
	ItemComponents.Add(NewItemComponent);

	// testqn
	InstanceID = FGuid::NewGuid();

	OnRep_ItemUpdated();
}

void UItemInstance::SetOwnerObject(UObject* InOwnerObject)
{
	OwnerObject = InOwnerObject;
	OwnerInventory = Cast<UInventoryData>(OwnerObject);

	OnRep_SetOwner();
}

const TArray<TObjectPtr<UItemComponentBase>>& UItemInstance::GetItemComponents() const
{
	return ItemComponents;
}

void UItemInstance::SetOwnerCharacter(ACharacter* InOwnerCharacter)
{
	OwnerCharacter = InOwnerCharacter;

	OnRep_SetOwnerCharacter();
}

bool UItemInstance::HasAuthority() const
{
	AActor* Owner = GetTypedOuter<AActor>();
	return IsValid(Owner) == true ? Owner->HasAuthority() : false;
}

void UItemInstance::CancelAllComponentActions()
{
	for (const auto& Component : ItemComponents)
	{
		if (IsValid(Component) == false)
			continue;

		Component->CancelAction();
	}
}

bool UItemInstance::RemoveFromSource()
{
	UInventoryData* Inventory = GetOwnerInventory();
	if (IsValid(Inventory) == true)
	{
		return Inventory->RemoveItem(this);
	}

	if (AItemWrapperActor* ItemActor = GetTypedOuter<AItemWrapperActor>())
	{
		ItemActor->Destroy();
		return true;
	}

	if (OwnerCharacter.IsValid() == true)
	{
		UEquipComponent* EquipComponent = OwnerCharacter->FindComponentByClass<UEquipComponent>();
		if (IsValid(EquipComponent) == true)
		{
			EquipComponent->ServerRPC_UnequipItem(this, EUnequipType::Destroy);
			return true;
		}
	}

	return false;
}
