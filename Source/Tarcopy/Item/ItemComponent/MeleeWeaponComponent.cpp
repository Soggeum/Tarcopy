#include "Item/ItemComponent/MeleeWeaponComponent.h"
#include "Item/DataTableSubsystem.h"
#include "Item/Data/MeleeWeaponData.h"
#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimMontage.h"
#include "FirearmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Item/ItemCommand/EquipCommand.h"
#include "Item/ItemComponent/DurabilityComponent.h"
#include "Character/MyCharacter.h"
#include "HoldableComponent.h"
#include "Tarcopy.h"
#include "Item/EquipComponent.h"
#include "Inventory/PlayerInventoryComponent.h"
#include "Inventory/InventoryData.h"
#include "Item/Data/ItemSoundData.h"

const float UMeleeWeaponComponent::CheckHitDelay = 0.5f;

void UMeleeWeaponComponent::SetOwnerItem(UItemInstance* InOwnerItem)
{
	Super::SetOwnerItem(InOwnerItem);
}

void UMeleeWeaponComponent::GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context)
{
	Super::GetCommands(OutCommands, Context);

	const FItemData* OwnerItemData = GetOwnerItemData();
	checkf(OwnerItemData != nullptr, TEXT("Owner Item has No Data"));
	FText TextItemName = OwnerItemData->TextName;

	if (Context.Instigator.IsValid() == false)
		return;

	if (GetData() == nullptr)
		return;

	UEquipComponent* EquipComponent = Context.Instigator->FindComponentByClass<UEquipComponent>();
	UPlayerInventoryComponent* InventoryComponent = Context.Instigator->FindComponentByClass<UPlayerInventoryComponent>();
	if (IsValid(EquipComponent) == false || IsValid(InventoryComponent) == false)
		return;

	bool bIsEquipped = IsValid(GetOwnerCharacter()) == true;
	UEquipCommand* EquipCommand = NewObject<UEquipCommand>(this);
	EquipCommand->TargetItem = GetOwnerItem();
	EquipCommand->TextDisplay = FText::Format(
		FText::FromString(bIsEquipped == true ? TEXT("Unequip {0}") : TEXT("Equip {0}")),
		TextItemName);
	EquipCommand->bEquip = !bIsEquipped;
	EquipCommand->BodyLocation = Data->BodyLocation;
	// 인벤토리에서 공간 있는지 체크해야 함
	UInventoryData* InventoryData = InventoryComponent->GetPlayerInventoryData();
	FIntPoint Origin;
	bool bRotated;
	if (bIsEquipped == true)
	{
		EquipCommand->bExecutable = IsValid(InventoryData) == true && InventoryData->CanAddItem(OwnerItem.Get(), Origin, bRotated) == true;
	}
	else
	{
		TArray<UItemInstance*> ReplaceItems;
		EquipComponent->GetNeedToReplace(Data->BodyLocation, ReplaceItems);
		if (ReplaceItems.Num() == 1)
		{
			EquipCommand->bExecutable = IsValid(InventoryData) == true && InventoryData->CanAddItem(OwnerItem.Get(), Origin, bRotated, ReplaceItems[0]) == true;
		}
		else
		{
			EquipCommand->bExecutable = true;
		}
	}
	OutCommands.Add(EquipCommand);
}

void UMeleeWeaponComponent::SetOwnerHoldingItemMesh()
{
	if (GetData() == nullptr)
		return;

	SetOwnerHoldingItemMeshAtSocket(Data->Socket);
}

void UMeleeWeaponComponent::SetOwnerAnimPreset()
{
	if (GetData() == nullptr)
		return;

	SetOwnerAnimPresetByHoldableType(Data->HoldableType);
}

void UMeleeWeaponComponent::OnExecuteAttack(const FVector& TargetLocation)
{
	if (GetData() == nullptr)
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return;

	UCharacterMovementComponent* CharacterMovement = OwnerCharacter->GetCharacterMovement();
	if (IsValid(CharacterMovement) == false)
		return;

	bIsAttacking = true;

	NetMulticast_PlayAttackMontage();

	float AttackDuration = IsValid(Data->Montage) == true ? Data->Montage->GetPlayLength() : 1.0f;
	CharacterMovement->DisableMovement();
	OwnerCharacter->GetWorldTimerManager().SetTimer(
		EnableMovementTimerHandle,
		this,
		&ThisClass::EnableOwnerMovement,
		AttackDuration,
		false);

	OwnerCharacter->GetWorldTimerManager().SetTimer(
		CheckHitTimerHandle,
		this,
		&ThisClass::CheckHit,
		AttackDuration * CheckHitDelay,
		false);
}

void UMeleeWeaponComponent::CancelAction()
{
	UWorld* World = GetWorld();
	if (IsValid(World) == true)
	{
		World->GetTimerManager().ClearTimer(CheckHitTimerHandle);
		World->GetTimerManager().ClearTimer(EnableMovementTimerHandle);
	}

	NetMulticast_StopAttackMontage();
	EnableOwnerMovement();
}

void UMeleeWeaponComponent::BeginDestroy()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::BeginDestroy();
}

void UMeleeWeaponComponent::OnRep_SetComponent()
{
	Super::OnRep_SetComponent();

	SetData();
}

void UMeleeWeaponComponent::CheckHit()
{
	if (GetData() == nullptr)
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return;

	NetMulticast_SoundAttack();

	HitActors.Empty();

	float AttackRadius = (Data->MaxRange - Data->MinRange) * 0.5f;
	float AttackOriginDistance = Data->MinRange + AttackRadius;
	FVector OwnerLocation = OwnerCharacter->GetActorLocation();
	FVector AttackOrigin = OwnerLocation + OwnerCharacter->GetActorForwardVector() * AttackOriginDistance;

	float MinRangeSquared = FMath::Square(Data->MinRange);
	float MaxRangeSquared = FMath::Square(Data->MaxRange);

	TArray<FHitResult> HitResults;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter);
	Params.bTraceComplex = true;
	Params.bReturnPhysicalMaterial = true;
	FCollisionShape CapsuleCollision = FCollisionShape::MakeCapsule(AttackRadius, 100.0f);

	bool bHit = GetWorld()->SweepMultiByChannel(HitResults, AttackOrigin, AttackOrigin, FQuat::Identity, ECC_PlayerAttack, CapsuleCollision, Params);
	for (const auto& HitResult : HitResults)
	{
		if (HitResult.bBlockingHit == false)
			continue;

		AActor* HitActor = HitResult.GetActor();
		if (IsValid(HitActor) == false)
			continue;

		if (HitActors.Contains(HitActor) == true)
			continue;

		if (HitResult.Component.IsValid() == true && HitResult.Component->GetCollisionObjectType() == ECC_WorldStatic)
			continue;

		if (CheckIsAttackableTarget(HitActor) == false)
			continue;

		float DistSquared = FVector::DistSquared(OwnerLocation, HitActor->GetActorLocation());
		float Damage = FMath::GetMappedRangeValueClamped(
			FVector2D(MinRangeSquared, MaxRangeSquared),
			FVector2D(Data->MinDamage, Data->MaxDamage),
			DistSquared);

		FVector Dir = (HitActor->GetActorLocation() - OwnerLocation).GetSafeNormal();
		UGameplayStatics::ApplyPointDamage(
			HitActor,
			Damage,
			Dir,
			HitResult,
			OwnerCharacter->GetController(),
			OwnerCharacter,
			nullptr);

		HitActors.Add(HitActor);
	}

	if (HitActors.IsEmpty() == false)
	{
		UDurabilityComponent* DurabilityComponent = GetOwnerItem()->GetItemComponent<UDurabilityComponent>();
		if (IsValid(DurabilityComponent) == true)
		{
			DurabilityComponent->LoseDurability(Data->LoseCondition);
		}
	}

	// 충돌 검사 디버그
	{
		DrawDebugCapsule(
			GetWorld(),
			AttackOrigin,
			100.0f,
			AttackRadius,
			FQuat::Identity,
			FColor::Green,
			false,
			5.0f);
	}

	/*for (const auto& HitEnemy : HitEnemies)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("Hit: %s"), *HitEnemy->GetName()));
	}*/
}

bool UMeleeWeaponComponent::CheckIsAttackableTarget(AActor* TargetActor)
{
	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return false;

	FVector Origin = OwnerCharacter->GetActorLocation();
	FVector TargetLocation = TargetActor->GetActorLocation();

	FHitResult HitResult;
	return !GetWorld()->LineTraceSingleByObjectType(HitResult, Origin, TargetLocation, ECC_WorldStatic);
}

void UMeleeWeaponComponent::NetMulticast_PlayAttackMontage_Implementation()
{
	if (GetData() == nullptr)
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return;

	if (IsValid(Data->Montage) == false)
		return;

	OwnerCharacter->PlayAnimMontage(Data->Montage);
}

void UMeleeWeaponComponent::NetMulticast_StopAttackMontage_Implementation()
{
	if (GetData() == nullptr)
		return;

	ACharacter* OwnerCharacter = GetOwnerCharacter();
	if (IsValid(OwnerCharacter) == false)
		return;

	if (IsValid(Data->Montage) == false)
		return;

	OwnerCharacter->StopAnimMontage(Data->Montage);
}

void UMeleeWeaponComponent::SetData()
{
	const FItemData* ItemData = GetOwnerItemData();
	if (ItemData == nullptr)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	Data = DataTableSubsystem->GetTable(EDataTableType::MeleeWeaponTable)->FindRow<FMeleeWeaponData>(ItemData->ItemId, FString(""));
}

void UMeleeWeaponComponent::NetMulticast_SoundAttack_Implementation()
{
	AMyCharacter* MyCharacter = Cast<AMyCharacter>(GetOwnerCharacter());
	if (IsValid(MyCharacter) == false)
		return;

	UDataTableSubsystem* DataTableSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDataTableSubsystem>();
	if (IsValid(DataTableSubsystem) == false)
		return;

	FItemSoundData* SoundData = DataTableSubsystem->GetTable(EDataTableType::ItemSoundTable)->FindRow<FItemSoundData>(TEXT("SFXMeleeWeaponR"), FString(""));
	if (SoundData == nullptr)
		return;

	FVector Location = MyCharacter->GetActorLocation();
	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundData->Sound,
		Location);
}

const FMeleeWeaponData* UMeleeWeaponComponent::GetData()
{
	if (Data == nullptr)
	{
		SetData();
	}
	return Data;
}
