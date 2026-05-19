#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ItemSpawnSubsystem.generated.h"
UCLASS()
class TARCOPY_API UItemSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	class AItemWrapperActor* SpawnItemAtGround(AActor* Instigator, const FName& ItemId);
	class AItemWrapperActor* SpawnItemAtGround(AActor* Instigator, class UItemInstance* ItemInstance);

private:
	const float SpawnYawOffset = 60.0f;
};
