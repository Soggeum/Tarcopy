#include "Item/ItemSpawnSubsystem.h"
#include "Item/ItemWrapperActor/ItemWrapperActor.h"
#include "Item/ItemInstance.h"

AItemWrapperActor* UItemSpawnSubsystem::SpawnItemAtGround(AActor* Instigator, const FName& ItemId)
{
	UItemInstance* ItemInstance = NewObject<UItemInstance>(this);
	ItemInstance->SetItemId(ItemId);

	return SpawnItemAtGround(Instigator, ItemInstance);
}

AItemWrapperActor* UItemSpawnSubsystem::SpawnItemAtGround(AActor* Instigator, UItemInstance* ItemInstance)
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false)
		return nullptr;

	// 서버에서만 스폰 가능
	if (World->GetNetMode() == NM_Client)
		return nullptr;

	if (IsValid(Instigator) == false)
		return nullptr;

	if (IsValid(ItemInstance) == false)
		return nullptr;

	FRotator SpawnDirRot = Instigator->GetActorForwardVector().Rotation();
	float RandomYaw = FMath::FRandRange(-SpawnYawOffset, SpawnYawOffset);
	SpawnDirRot.Yaw += RandomYaw;
	FVector SpawnDirOffset = SpawnDirRot.Vector();
	FVector SpawnLocation = Instigator->GetActorLocation() + SpawnDirOffset * 40.0f;
	FVector Start = SpawnLocation + FVector(0.0f, 0.0f, 50.0f);
	FVector End = SpawnLocation - FVector(0.0f, 0.0f, 200.0f);

	FHitResult HitResult;
	if (World->LineTraceSingleByObjectType(HitResult, Start, End, ECC_WorldStatic) == true)
	{
		SpawnLocation = HitResult.Location;
		FRotator SpawnRotation = FRotator::ZeroRotator;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AItemWrapperActor* ItemWrapperActor = GetWorld()->SpawnActor<AItemWrapperActor>(
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams);

	if (IsValid(ItemWrapperActor) == true)
	{
		ItemWrapperActor->SetItemInstance(ItemInstance);
		return ItemWrapperActor;
	}

	return nullptr;
}
