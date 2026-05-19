
#include "Car/Component/TCCarCombatComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "WheeledVehiclePawn.h"
#include "Car/TCCarBase.h"
#include "Car/TCChaosVehicleDummyWheel.h"
#include "Components/BoxComponent.h"
#include "Car/UI/TCCarWidget.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Character/MyCharacter.h"
#include "Controller/MyPlayerController.h"

UTCCarCombatComponent::UTCCarCombatComponent() :
	DamageFactor(0.00001),
	MinDamageImpulse(50000.f)
{
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = false;

	FrontBox = CreateDefaultSubobject<UBoxComponent>(TEXT("FrontBox"));

	FrontBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FrontBox->ComponentTags.Add("Front");
	DamageZone.Add(FrontBox);

	BackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BackBox"));
	BackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BackBox->ComponentTags.Add("Back");
	DamageZone.Add(BackBox);

	RightBox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightBox"));
	RightBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightBox->ComponentTags.Add("Right");
	DamageZone.Add(RightBox);

	LeftBox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftBox"));
	LeftBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftBox->ComponentTags.Add("Left");
	DamageZone.Add(LeftBox);

}

void UTCCarCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();

	if (AWheeledVehiclePawn* Vehicle = Cast<AWheeledVehiclePawn>(Owner))
	{
		if (USkeletalMeshComponent* VehicleMesh = Vehicle->GetMesh())
		{
			VehicleMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			VehicleMesh->SetNotifyRigidBodyCollision(true);
			if (GetOwner()->HasAuthority())
			{
				VehicleMesh->OnComponentHit.AddDynamic(this, &UTCCarCombatComponent::OnVehicleHit);
			}
		}
	}

	Owner->GetComponents<UStaticMeshComponent>(Meshes);
	checkf(PartDataAsset, TEXT("No Data"));

	for (UStaticMeshComponent* Mesh : Meshes)
	{
		Mesh->SetIsReplicated(true);
		const FName CompName = Mesh->GetFName();

		if (const FCarPartStat* Stat =
			PartDataAsset->PartData.Find(CompName))
		{
			PartDataMap.Add(Mesh, *Stat);
			/*ComponentHealth.Add(Mesh, Stat->MaxHealth);*/

			ComponentName.Add(CompName, Mesh);
			FCarPartHP Temp;
			Temp.PartHP = Stat->MaxHealth;
			Temp.PartName = CompName;
			PartsHP.Add(Temp);
		}
	}
}

void UTCCarCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PartsHP);
}

void UTCCarCombatComponent::DestroyPart(UPrimitiveComponent* DestroyComponent)
{
	MulticastCarPlaySound(CrashSound);
	if (DestroyComponent->ComponentHasTag("Window"))
	{
		DestroyWindow(DestroyComponent);
		return;
	}

	if (DestroyComponent->ComponentHasTag("Wheel"))
	{
		DestroyWheel(DestroyComponent);
		return;
	}

	if (DestroyComponent->ComponentHasTag("Main"))
	{
		DestroyMain(DestroyComponent);
		return;
	}

	DestroyDefault(DestroyComponent);
	return;
}

void UTCCarCombatComponent::DestroyWindow(UPrimitiveComponent* DestroyComponent)
{
	/*DestroyComponent->SetVisibility(false);
	DestroyComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/

	//glass effect spawn
	DestroyDefault(DestroyComponent);
}

void UTCCarCombatComponent::DestroyWheel(UPrimitiveComponent* DestroyComponent)
{
	DisableWheelPhysics(DestroyComponent);

	DestroyDefault(DestroyComponent);
}

void UTCCarCombatComponent::DestroyMain(UPrimitiveComponent* DestroyComponent)
{
	if (DestroyedMain) return;

	if (AWheeledVehiclePawn* Vehicle = Cast<AWheeledVehiclePawn>(GetOwner()))
	{
		if (USkeletalMeshComponent* VehicleMesh = Vehicle->GetMesh())
		{
			if (GetOwner()->HasAuthority())
			{
				VehicleMesh->OnComponentHit.RemoveDynamic(this, &UTCCarCombatComponent::OnVehicleHit);
			}
		}
	}
	DestroyedMain = true;
	for (auto &Mesh : Meshes)
	{
		DestroyPart(Mesh);
	}
	

	ATCCarBase* Car = Cast<ATCCarBase>(GetOwner());
	if (!Car) return;
	Car->bCanRide = false;
	Car->bEngineOn = false;
	AMyPlayerController* PC = Cast<AMyPlayerController>(Car->GetController());
	if (PC)
	{
		PC->Possess(Car->DriverPawn);
	}
	for (auto Passenger : Car->Passengers)
	{
		ClientRPCRequestExit(Car, Passenger, Cast<APlayerController>(Passenger->GetController()));
		UGameplayStatics::ApplyPointDamage(
			Passenger,  
			500.f,            
			GetOwner()->GetActorForwardVector(),
			FHitResult(),     
			GetOwner()->GetInstigatorController(),
			GetOwner(),                
			UDamageType::StaticClass()
		);
	}

	MulticastCarPlaySound(ExplosionSound);
}

void UTCCarCombatComponent::DestroyDefault(UPrimitiveComponent* DestroyComponent)
{
	DestroyComponent->SetCollisionProfileName("BlockAll");
	DestroyComponent->SetSimulatePhysics(true);
	DestroyComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
}

void UTCCarCombatComponent::DisableWheelPhysics(UPrimitiveComponent* DestroyComponent)
{
	ATCCarBase* Car = Cast<ATCCarBase>(GetOwner());
	if (!Car) return;


	Car->DisableWheel(DestroyComponent);

	/*Move->SetWheelFrictionMultiplier(WheelIndex, 0.1f);
	Move->SetWheelHandbrakeTorque(WheelIndex, 0.1f);
	Move->SetWheelMaxBrakeTorque(WheelIndex, 0.f);
	Move->SetWheelMaxSteerAngle(WheelIndex, 0.1f);
	Move->SetWheelRadius(WheelIndex, 10.f);
	Move->SetWheelSlipGraphMultiplier(WheelIndex, 0.1f);

	Move->SetSuspensionParams(0.f, 0.f, 0.f, 0.f, 0.f, WheelIndex);*///이방법은 너무 연산을 많이먹음 바퀴가 부서지면 차체에서 인풋값을 절반을받거나 하는 방식으로 해야할듯..? 기우는 걸 해결할 방법이 없기는 함.. 이거하니까 차체가 날아감

}

void UTCCarCombatComponent::OnVehicleHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

	if (!GetOwner()) return;
	if (OtherActor == GetOwner()) return;

	const float Now = GetWorld()->GetTimeSeconds();

	if (Now - LastHitTime < 0.5f) return;

	LastHitTime = Now;

	float Damage = 0.f;
	if (ACharacter* HitActor = Cast<ACharacter>(OtherActor))
	{
		ATCCarBase* Car = Cast<ATCCarBase>(GetOwner());
		if (!Car) return;

		float Speed = Car->GetChaosVehicleMovement()->GetForwardSpeed();
		float SpeedKmh = Speed * 0.036f;
		FVector Dir = (HitActor->GetActorLocation() - GetOwner()->GetActorLocation());
		Dir.Z += 100.f;
		Dir = Dir.GetSafeNormal();

		if (SpeedKmh <= 15.f) return;

		HitActor->LaunchCharacter(Dir * SpeedKmh * 40, true, true);
		Damage = 5.f;

		UGameplayStatics::ApplyPointDamage(
			HitActor,
			SpeedKmh,
			GetOwner()->GetVelocity().GetSafeNormal(),
			Hit,
			GetOwner()->GetInstigatorController(),
			GetOwner(),
			UDamageType::StaticClass()
		);	
	}
	else
	{
		const float ImpulseSize = NormalImpulse.Size();

		if (ImpulseSize < MinDamageImpulse)
			return;

		Damage = ImpulseSize * DamageFactor;
	}

	const FVector WorldPoint = Hit.ImpactPoint;


	for (auto Zone : DamageZone)
	{
		if (Zone && IsPointInsideBox(Zone, WorldPoint))
		{
			ApplyDamage(Zone, Damage, WorldPoint);
		}
	}
}

void UTCCarCombatComponent::ApplyDamage(UBoxComponent* InBox, float Damage, const FVector& WorldPoint)
{
	if (!GetOwner()->HasAuthority()) return;
	if (!InBox) return;

	MulticastCarPlaySound(HitSound);

	for (const FName& Tag : InBox->ComponentTags)
	{
		for (FCarPartHP& Part : PartsHP)
		{
			if (ComponentName[Part.PartName]->ComponentHasTag(Tag))
			{
				/*FVector MeshLocation = MeshHP.Key->GetComponentLocation();
				float MeshImpactDist = FVector::Dist(MeshLocation, WorldPoint);*/
				Part.PartHP = FMath::Clamp(Part.PartHP - Damage, 0.f, PartDataMap[ComponentName[Part.PartName]].MaxHealth);


				if (Part.PartHP <= 0)
				{
					DestroyPart(ComponentName[Part.PartName]);
					Part.bIsDestroyed = true;
				}


				/*if (ComponentName[Part.PartName]->ComponentHasTag("Main"))
				{
					ATCCarBase* Car = Cast<ATCCarBase>(GetOwner());
					if (!Car) return;

					UTCCarWidget* WidgetInstance = Car->CarWidgetInstance;
					WidgetInstance->UpdateCarDamage(Part.PartHP / PartDataMap[ComponentName[Part.PartName]].MaxHealth);
				}*/
			}
		}
	}
}

bool UTCCarCombatComponent::IsPointInsideBox(UBoxComponent* InBox, const FVector& WorldPoint)
{
	if (!InBox) return false;

	const FTransform& BoxTM = InBox->GetComponentTransform();
	FVector LocalPoint = BoxTM.InverseTransformPosition(WorldPoint);
	FVector Extent = InBox->GetUnscaledBoxExtent();

	return
		FMath::Abs(LocalPoint.X) <= Extent.X &&
		FMath::Abs(LocalPoint.Y) <= Extent.Y &&
		FMath::Abs(LocalPoint.Z) <= Extent.Z;

}

UPrimitiveComponent* UTCCarCombatComponent::GetTestMesh()
{
	int32 RandIndex = FMath::RandRange(0, Meshes.Num() - 1);
	TestMesh = Meshes[RandIndex];
	return TestMesh;
}

void UTCCarCombatComponent::ClientRPCRequestExit_Implementation(APawn* InCar, APawn* InPawn, APlayerController* InPC)
{
	ATCCarBase* Car = Cast<ATCCarBase>(InCar);
	if (!Car) return;
	Car->ExitVehicle(InPawn, InPC);
}

void UTCCarCombatComponent::MulticastCarPlaySound_Implementation(USoundBase* NewSound)
{
	if (!NewSound || !GetOwner()) return;
	UGameplayStatics::PlaySoundAtLocation(
		GetOwner()->GetWorld(),
		NewSound,
		GetOwner()->GetActorLocation()
	);
}

