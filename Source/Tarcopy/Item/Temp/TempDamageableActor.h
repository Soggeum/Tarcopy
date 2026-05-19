#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TempDamageableActor.generated.h"

UCLASS()
class TARCOPY_API ATempDamageableActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ATempDamageableActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UHealthComponent> HealthComponent;
};
