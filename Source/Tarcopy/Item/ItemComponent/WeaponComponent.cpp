#include "Item/ItemComponent/WeaponComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsAttacking);
}

void UWeaponComponent::ExecuteAttack(const FVector& TargetLocation)
{
	if (bIsAttacking == true)
		return;

	OnExecuteAttack(TargetLocation);
}

void UWeaponComponent::EnableOwnerMovement()
{
	if (bIsAttacking == false)
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return;

	UCharacterMovementComponent* CharacterMovement = OwnerCharacter->GetCharacterMovement();
	if (IsValid(CharacterMovement) == false)
		return;

	bIsAttacking = false;
	CharacterMovement->SetMovementMode(EMovementMode::MOVE_Walking);
}
