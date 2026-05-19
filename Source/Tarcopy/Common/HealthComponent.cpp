#include "Common/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Common/BodyDamageModifierSetting.h"
#include "Item/EquipComponent.h"
#include "Character/MyCharacter.h"
#include "Controller/MyPlayerController.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
		
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, MaxHP);
	DOREPLIFETIME(ThisClass, CurrentHP);
}

float UHealthComponent::TakeDamage(float Damage, const FHitResult& HitResult)
{
	float ActualDamage = Damage;

	if (bIsDead)
	{
		return ActualDamage;
	}

	UPhysicalMaterial* PhysMat = HitResult.PhysMaterial.IsValid() == true ? HitResult.PhysMaterial.Get() : nullptr;
	if (IsValid(PhysMat) == true)
	{
		if (IsValid(BodyDamageSetting) == true)
		{
			float* PtrBodyDamageModifier = BodyDamageSetting->BodyMap.Find(PhysMat);
			if (PtrBodyDamageModifier != nullptr)
			{
				ActualDamage = ActualDamage *= *PtrBodyDamageModifier;
			}
		}

		UEquipComponent* EquipComponent = GetOwner()->FindComponentByClass<UEquipComponent>();
		if (IsValid(EquipComponent) == true)
		{
			ActualDamage *= EquipComponent->GetFinalDamageTakenMultiplier(PhysMat);
		}
	}

	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.0f, MaxHP);
	if (CurrentHP <= 0.0f)
	{
		if (OnDead.IsBound() == true)
		{
			OnDead.Broadcast();
			bIsDead = true;
		}
	}

	OnRep_PrintHP();

	return ActualDamage;
}

void UHealthComponent::RestoreHP(float InHP)
{
	if (bIsDead == true)
		return;

	CurrentHP = FMath::Clamp(CurrentHP + InHP, 0.0f, MaxHP);
	OnRep_PrintHP();
}

void UHealthComponent::OnRep_PrintHP()
{
	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	AMyPlayerController* PC = IsValid(Owner) == true ? Owner->GetController<AMyPlayerController>() : nullptr;
	if (IsValid(PC) == false)
	{
		return;
	}

	PC->SetHealthUI(CurrentHP, MaxHP);
}
