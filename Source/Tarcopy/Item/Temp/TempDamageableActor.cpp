#include "Item/Temp/TempDamageableActor.h"
#include "Common/HealthComponent.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

ATempDamageableActor::ATempDamageableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ATempDamageableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority() == true)
	{
		if (IsValid(HealthComponent) == true)
		{
			HealthComponent->OnDead.AddLambda
			([WeakThis = TWeakObjectPtr(this)]()-> void
			{
				if (WeakThis.IsValid() == true)
				{
					WeakThis->Destroy();
				}
			});
		}
	}
}

void ATempDamageableActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float ATempDamageableActor::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (IsValid(HealthComponent) == true)
	{
		const FPointDamageEvent* PointDamageEvent = (const FPointDamageEvent*)(&DamageEvent);
		if (PointDamageEvent != nullptr)
		{
			HealthComponent->TakeDamage(Damage, PointDamageEvent->HitInfo);
		}
	}
	return 0.0f;
}
