#include "Item/ItemComponent/HoldableComponent.h"
#include "Character/MyCharacter.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemEnums.h"
#include "Net/UnrealNetwork.h"

void UHoldableComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsHolding);
}

void UHoldableComponent::SetHolding(bool bInIsHolding)
{
	bIsHolding = bInIsHolding;
	OnRep_Holding();
}

void UHoldableComponent::OnRep_Holding()
{
	if (bIsHolding == true)
	{
		SetOwnerHoldingItemMesh();
		SetOwnerAnimPreset();
	}
}

void UHoldableComponent::SetOwnerHoldingItemMeshAtSocket(EHoldableSocket Socket)
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwnerCharacter());
	if (IsValid(OwnerCharacter) == false)
		return;

	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	static const TMap<EHoldableSocket, FName> SocketNameMap =
	{
		{ EHoldableSocket::RightHand, FName("hand_rSocket")},
		{ EHoldableSocket::LeftHand, FName("hand_lSocket")}
	};

	const FName* SocketName = SocketNameMap.Find(Socket);
	if (SocketName == nullptr)
		return;

	OwnerCharacter->SetHoldingItemMesh(ItemData->DefaultMesh, *SocketName);
}

void UHoldableComponent::SetOwnerAnimPresetByHoldableType(EHoldableType Type)
{
	AMyCharacter* OwnerCharacter = Cast<AMyCharacter>(GetOwnerCharacter());
	if (IsValid(OwnerCharacter) == false)
		return;

	OwnerCharacter->SetAnimPreset(Type);
}
