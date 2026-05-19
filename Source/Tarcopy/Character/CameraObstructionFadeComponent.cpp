#include "Character/CameraObstructionFadeComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "CollisionShape.h"

UCameraObstructionFadeComponent::UCameraObstructionFadeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraObstructionFadeComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* OwnerActor = GetOwner())
	{
		CachedCamera = OwnerActor->FindComponentByClass<UCameraComponent>();
		CachedCapsule = OwnerActor->FindComponentByClass<UCapsuleComponent>();
	}
}

void UCameraObstructionFadeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastTrace += DeltaTime;
	if (TimeSinceLastTrace >= TraceInterval)
	{
		TimeSinceLastTrace = 0.f;
		UpdateObstructors();
	}

	ApplyFade(DeltaTime);
}

void UCameraObstructionFadeComponent::UpdateObstructors()
{
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor) || !IsValid(GetWorld()))
	{
		return;
	}

	UCameraComponent* Camera = CachedCamera.Get();
	if (!IsValid(Camera))
	{
		return;
	}

	const FVector Start = Camera->GetComponentLocation();
	FVector TargetLocation = OwnerActor->GetActorLocation();
	if (UCapsuleComponent* Capsule = CachedCapsule.Get())
	{
		TargetLocation.Z += Capsule->GetScaledCapsuleHalfHeight();
	}
	if (!FMath::IsNearlyZero(OwnerZOffset))
	{
		TargetLocation.Z += OwnerZOffset;
	}

	PreviousObstructors = CurrentObstructors;
	CurrentObstructors.Reset();

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CameraObstruction), /*bTraceComplex*/ false);
	Params.AddIgnoredActor(OwnerActor);

	TArray<FHitResult> Hits;
	const FCollisionShape SphereShape = FCollisionShape::MakeSphere(TraceRadius);
	if (GetWorld()->SweepMultiByChannel(Hits, Start, TargetLocation, FQuat::Identity, TraceChannel, SphereShape, Params))
	{
		for (const FHitResult& Hit : Hits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!IsValid(HitActor) || HitActor == OwnerActor)
			{
				continue;
			}

			CurrentObstructors.Add(HitActor);
		}
	}

	// Actors newly obstructing: fade out.
	for (const TWeakObjectPtr<AActor>& ActorPtr : CurrentObstructors)
	{
		if (ActorPtr.IsValid() && !PreviousObstructors.Contains(ActorPtr))
		{
			StartFadeForActor(ActorPtr.Get(), 0.f);
		}
	}

	// Actors no longer obstructing: fade in.
	for (const TWeakObjectPtr<AActor>& ActorPtr : PreviousObstructors)
	{
		if (ActorPtr.IsValid() && !CurrentObstructors.Contains(ActorPtr))
		{
			StartFadeForActor(ActorPtr.Get(), 1.f);
		}
	}
}

void UCameraObstructionFadeComponent::ApplyFade(float DeltaTime)
{
	for (auto It = MeshStates.CreateIterator(); It; ++It)
	{
		UMeshComponent* Mesh = It.Key().Get();
		FMeshFadeState& State = It.Value();

		if (!IsValid(Mesh))
		{
			It.RemoveCurrent();
			continue;
		}

		const bool bFadingOut = State.TargetOpacity < State.CurrentOpacity;
		const float FadeTime = bFadingOut ? FadeOutTime : FadeInTime;
		const float Speed = (FadeTime > KINDA_SMALL_NUMBER) ? (1.f / FadeTime) : 9999.f;

		State.CurrentOpacity = FMath::FInterpTo(State.CurrentOpacity, State.TargetOpacity, DeltaTime, Speed);
		State.CurrentOpacity = FMath::Clamp(State.CurrentOpacity, 0.f, 1.f);

		SetMeshOpacity(Mesh, State.CurrentOpacity);

		// When fully opaque and target is 1, restore originals and remove state.
		if (FMath::IsNearlyEqual(State.CurrentOpacity, 1.f, 0.01f) && FMath::IsNearlyEqual(State.TargetOpacity, 1.f, 0.01f))
		{
			const TArray<UMaterialInterface*>& Originals = State.OriginalMaterials;
			for (int32 Index = 0; Index < Originals.Num(); ++Index)
			{
				Mesh->SetMaterial(Index, Originals[Index]);
			}
			It.RemoveCurrent();
		}
	}
}

void UCameraObstructionFadeComponent::StartFadeForActor(AActor* Actor, float TargetOpacity)
{
	if (!IsValid(Actor))
	{
		return;
	}

	TArray<UMeshComponent*> Meshes;
	Actor->GetComponents<UMeshComponent>(Meshes);

	for (UMeshComponent* Mesh : Meshes)
	{
		if (!IsValid(Mesh))
		{
			continue;
		}

		FMeshFadeState& State = MeshStates.FindOrAdd(Mesh);

		// Cache originals once.
		if (State.OriginalMaterials.Num() == 0)
		{
			State.OriginalMaterials = Mesh->GetMaterials();
			State.CurrentOpacity = 1.f;
		}

		// Ensure DMIs exist.
		for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
		{
			if (!Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(Index)))
			{
				Mesh->CreateAndSetMaterialInstanceDynamic(Index);
			}
		}

		State.TargetOpacity = FMath::Clamp(TargetOpacity, 0.f, 1.f);
	}
}

void UCameraObstructionFadeComponent::SetMeshOpacity(UMeshComponent* Mesh, float NewOpacity)
{
	if (!IsValid(Mesh))
	{
		return;
	}

	for (int32 Index = 0; Index < Mesh->GetNumMaterials(); ++Index)
	{
		if (UMaterialInstanceDynamic* DMI = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(Index)))
		{
			DMI->SetScalarParameterValue(OpacityParamName, NewOpacity);
		}
	}
}
