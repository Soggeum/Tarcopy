#include "Character/MoodleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Character/MyCharacter.h"
#include "Controller/MyPlayerController.h"
#include "GameFramework/GameStateBase.h"

UMoodleComponent::UMoodleComponent() :
	HungerReduceDelay(2.0f),
	HungerReduceAmount(1.0f),
	ThirstReduceDelay(2.0f),
	ThirstReduceAmount(1.0f),
	MaxHunger(100.0f),
	CurrentHunger(100.0f),
	MaxThirst(100.0f),
	CurrentThirst(100.0f),
	MaxStamina(100.0f),
	CurrentStamina(100.0f)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMoodleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner()->HasAuthority() == false)
		return;

	GetWorld()->GetTimerManager().SetTimer(
		HungerTimerHandle,
		this,
		&ThisClass::ReduceHungerByTime,
		HungerReduceDelay,
		true);

	GetWorld()->GetTimerManager().SetTimer(
		ThirstTimerHandle,
		this,
		&ThisClass::ReduceThirstByTime,
		ThirstReduceDelay,
		true);
}

void UMoodleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentHunger);
	DOREPLIFETIME(ThisClass, CurrentThirst);
	DOREPLIFETIME(ThisClass, CurrentStamina);
}

void UMoodleComponent::RestoreHunger(float InHunger)
{
	if (GetOwner()->HasAuthority() == false)
		return;

	SetHunger(CurrentHunger + InHunger);
}

void UMoodleComponent::RestoreThirst(float InThirst)
{
	if (GetOwner()->HasAuthority() == false)
		return;

	SetThirst(CurrentThirst + InThirst);
}

void UMoodleComponent::RestoreStamina(float InStamina)
{
	if (GetOwner()->HasAuthority() == false)
		return;

	SetStamina(CurrentStamina + InStamina);
}

void UMoodleComponent::ReduceHungerByTime()
{
	if (GetOwner()->HasAuthority() == false)
		return;

	SetHunger(CurrentHunger - HungerReduceAmount);
}

void UMoodleComponent::ReduceThirstByTime()
{
	if (GetOwner()->HasAuthority() == false)
		return;

	SetThirst(CurrentThirst - ThirstReduceAmount);
}

void UMoodleComponent::OnRep_SetHunger()
{
	UpdateHungerUI(CurrentHunger, MaxHunger);
}

void UMoodleComponent::OnRep_SetThirst()
{
	UpdateThirstUI(CurrentThirst, MaxThirst);
}

void UMoodleComponent::OnRep_SetStamina()
{
	UpdateStaminaUI(CurrentStamina, MaxStamina);
}

void UMoodleComponent::UpdateHungerUI(float CurrentValue, float MaxValue)
{
	if (GetOwner()->HasAuthority() == true)
		return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	AMyPlayerController* PC = IsValid(Owner) == true ? Owner->GetController<AMyPlayerController>() : nullptr;
	if (IsValid(PC) == false)
		return;

	PC->SetHungerTextUI(CurrentValue, MaxValue);
}

void UMoodleComponent::UpdateThirstUI(float CurrentValue, float MaxValue)
{
	if (GetOwner()->HasAuthority() == true)
		return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	AMyPlayerController* PC = IsValid(Owner) == true ? Owner->GetController<AMyPlayerController>() : nullptr;
	if (IsValid(PC) == false)
		return;

	PC->SetThirstTextUI(CurrentValue, MaxValue);
}

void UMoodleComponent::UpdateStaminaUI(float CurrentValue, float MaxValue)
{
	if (GetOwner()->HasAuthority() == true)
		return;

	ACharacter* Owner = Cast<ACharacter>(GetOwner());
	AMyPlayerController* PC = IsValid(Owner) == true ? Owner->GetController<AMyPlayerController>() : nullptr;
	if (IsValid(PC) == false)
		return;
}

void UMoodleComponent::SetHunger(float InHunger)
{
	CurrentHunger = FMath::Clamp(InHunger, 0, MaxStamina);
}

void UMoodleComponent::SetThirst(float InThirst)
{
	CurrentThirst = FMath::Clamp(InThirst, 0, MaxStamina);
}

void UMoodleComponent::SetStamina(float InStamina)
{
	CurrentStamina = FMath::Clamp(InStamina, 0, MaxStamina);
}
