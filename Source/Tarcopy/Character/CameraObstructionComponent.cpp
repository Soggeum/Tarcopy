// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/CameraObstructionComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "WorldCollision.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UCameraObstructionComponent::UCameraObstructionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCameraObstructionComponent::SetCamera(UCameraComponent* InCamera)
{
	Camera = InCamera;
}

void UCameraObstructionComponent::SetCapsule(UCapsuleComponent* InCapsule)
{
	Capsule = InCapsule;
}

void UCameraObstructionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
		return;
	}

	if (APawn* PawnOwner = Cast<APawn>(GetOwner()))
	{
		if (!PawnOwner->IsLocallyControlled())
		{
			SetComponentTickEnabled(false);
			return;
		}
	}

	if (!Camera)
	{
		Camera = GetOwner() ? GetOwner()->FindComponentByClass<UCameraComponent>() : nullptr;
	}
	if (!Capsule)
	{
		Capsule = GetOwner() ? GetOwner()->FindComponentByClass<UCapsuleComponent>() : nullptr;
	}
}

void UCameraObstructionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastObstructionTrace += DeltaTime;
	if (TimeSinceLastObstructionTrace >= ObstructionTraceInterval)
	{
		TimeSinceLastObstructionTrace = 0.f;
		UpdateCameraObstructionFade();
	}
}

void UCameraObstructionComponent::UpdateCameraObstructionFade()
{
	if (!IsValid(Camera) || !IsValid(GetWorld()))
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 0.f;
	const FVector CharacterCenter = OwnerActor->GetActorLocation() + FVector(0.f, 0.f, HalfHeight);

	// Restore materials whose hold time expired.
	for (auto It = FadeHoldUntil.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid() || Now > It.Value())
		{
			if (It.Key().IsValid())
			{
				ClearObstructionMaterial(It.Key().Get());
			}
			It.RemoveCurrent();
		}
	}

	const FVector Start = Camera->GetComponentLocation();
	const FVector End = CharacterCenter;
	const FVector TraceDirection = (End - Start).GetSafeNormal();

	FVector2D ScreenCenterUV = FVector2D::ZeroVector;
	float ScreenRadius = 0.f;
	float ScreenSoftness = 0.f;
	float ScreenAspect = 1.f;
	float ViewportHeight = 0.f;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		int32 ViewX = 0;
		int32 ViewY = 0;
		PC->GetViewportSize(ViewX, ViewY);
		if (ViewX > 0 && ViewY > 0)
		{
			FVector2D ScreenPos;
			if (PC->ProjectWorldLocationToScreen(CharacterCenter, ScreenPos, true))
			{
				ScreenCenterUV = FVector2D(ScreenPos.X / ViewX, ScreenPos.Y / ViewY);
				ScreenAspect = static_cast<float>(ViewX) / static_cast<float>(ViewY);
				ViewportHeight = static_cast<float>(ViewY);
				ScreenRadius = ScreenRadiusPx / ViewportHeight;
				ScreenSoftness = ScreenSoftnessPx / ViewportHeight;
			}
		}
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CameraOcclusion), /*bTraceComplex=*/ false);
	Params.AddIgnoredActor(OwnerActor);

	TSet<UPrimitiveComponent*> BlockingHitComps;
	TMap<UPrimitiveComponent*, float> ObstructionOpacityByComp;
	FVector TraceStart = Start;
	for (int32 HitIndex = 0; HitIndex < MaxObstructionHits; ++HitIndex)
	{
		FHitResult Hit;
		if (!GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, End, ECC_Visibility, Params))
		{
			break;
		}

		UPrimitiveComponent* HitComp = Hit.GetComponent();
		if (!IsValid(HitComp))
		{
			break;
		}

		// Skip overlaps with our own components.
		if (HitComp->GetOwner() == OwnerActor)
		{
			Params.AddIgnoredComponent(HitComp);
			TraceStart = Hit.ImpactPoint + TraceDirection * ObstructionTraceStep;
			continue;
		}

		BlockingHitComps.Add(HitComp);

		// Refresh hold time and apply hole on this surface.
		FadeHoldUntil.FindOrAdd(HitComp) = Now + FadeHoldTime;
		const float TargetOpacity = (HitIndex == 0) ? FirstObstructionOpacity : SubsequentObstructionOpacity;
		ObstructionOpacityByComp.Add(HitComp, TargetOpacity);
		ApplyObstructionMaterial(HitComp, Start, End, ScreenCenterUV, ScreenRadius, ScreenSoftness, ScreenAspect, TargetOpacity);

		Params.AddIgnoredComponent(HitComp);
		TraceStart = Hit.ImpactPoint + TraceDirection * ObstructionTraceStep;
		if (FVector::DistSquared(TraceStart, End) <= KINDA_SMALL_NUMBER)
		{
			break;
		}
	}

	if (bAffectComponentsInScreenRadius && ViewportHeight > 0.f)
	{
		const float DistanceToCharacter = FVector::Dist(Start, End);
		const float FovX = FMath::DegreesToRadians(Camera->FieldOfView);
		const float FovY = 2.f * FMath::Atan(FMath::Tan(FovX * 0.5f) / ScreenAspect);
		const float ViewHeightAtDistance = 2.f * DistanceToCharacter * FMath::Tan(FovY * 0.5f);
		const float CandidateRadius = ScreenCandidateRadiusScale * ScreenRadius * ViewHeightAtDistance;

		FCollisionObjectQueryParams ObjectParams;
		ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);

		TArray<FOverlapResult> Overlaps;
		const FCollisionShape Sphere = FCollisionShape::MakeSphere(CandidateRadius);
		if (GetWorld()->OverlapMultiByObjectType(Overlaps, End, FQuat::Identity, ObjectParams, Sphere, Params))
		{
			for (const FOverlapResult& Overlap : Overlaps)
			{
				UPrimitiveComponent* OverlapComp = Overlap.GetComponent();
				if (!IsValid(OverlapComp))
				{
					continue;
				}
				if (!BlockingHitComps.Contains(OverlapComp))
				{
					continue;
				}

				AActor* OverlapOwner = OverlapComp->GetOwner();
				if (!IsValid(OverlapOwner) || OverlapOwner == OwnerActor)
				{
					continue;
				}

				const FVector BoundsCenter = OverlapComp->Bounds.Origin;
				const FVector ToOverlap = BoundsCenter - Start;
				const float Along = FVector::DotProduct(ToOverlap, TraceDirection);
				if (Along <= 0.f)
				{
					continue;
				}

				FVector ClosestPoint = BoundsCenter;
				const float ClosestDist = OverlapComp->GetClosestPointOnCollision(Start, ClosestPoint);
				if (ClosestDist > 0.f)
				{
					if (ClosestDist > DistanceToCharacter)
					{
						continue;
					}
				}
				else
				{
					const float SphereRadius = OverlapComp->Bounds.SphereRadius;
					if (Along - SphereRadius > DistanceToCharacter)
					{
						continue;
					}
				}

				const float ViewHeightAtAlong = 2.f * Along * FMath::Tan(FovY * 0.5f);
				const float CandidateRadiusAtAlong = ScreenCandidateRadiusScale * ScreenRadius * ViewHeightAtAlong;
				const float DistToSegment = FMath::PointDistToSegment(ClosestPoint, Start, End);
				if (DistToSegment > CandidateRadiusAtAlong)
				{
					continue;
				}
				if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
				{
					FVector2D ScreenPos;
					if (!PC->ProjectWorldLocationToScreen(BoundsCenter, ScreenPos, true))
					{
						continue;
					}

					const float ViewX = ViewportHeight * ScreenAspect;
					const FVector2D UV(ScreenPos.X / ViewX, ScreenPos.Y / ViewportHeight);
					FVector2D Delta = UV - ScreenCenterUV;
					Delta.X *= ScreenAspect;

					const float Dist = Delta.Size();
					if (Dist <= ScreenRadius + ScreenSoftness)
					{
						FadeHoldUntil.FindOrAdd(OverlapComp) = Now + FadeHoldTime;
						const float* StoredOpacity = ObstructionOpacityByComp.Find(OverlapComp);
						const float TargetOpacity = StoredOpacity ? *StoredOpacity : SubsequentObstructionOpacity;
						ApplyObstructionMaterial(OverlapComp, Start, End, ScreenCenterUV, ScreenRadius, ScreenSoftness, ScreenAspect, TargetOpacity);
					}
				}
			}
		}
	}
}

void UCameraObstructionComponent::ApplyObstructionMaterial(UPrimitiveComponent* HitComp, const FVector& LineStart, const FVector& LineEnd,
	const FVector2D& ScreenCenterUV, float ScreenRadius, float ScreenSoftness, float ScreenAspect, float Opacity)
{
	if (!IsValid(HitComp))
	{
		return;
	}

	const int32 NumMaterials = HitComp->GetNumMaterials();
	if (NumMaterials <= 0)
	{
		return;
	}

	for (int32 Index = 0; Index < NumMaterials; ++Index)
	{
		UMaterialInterface* CurrentMat = HitComp->GetMaterial(Index);
	}

	bool bNeedsRebuild = false;
	TArray<UMaterialInstanceDynamic*>* DynMats = DynamicMaterials.Find(HitComp);
	if (!DynMats || DynMats->Num() != NumMaterials)
	{
		bNeedsRebuild = true;
	}
	else
	{
		for (UMaterialInstanceDynamic* DynMat : *DynMats)
		{
			if (!IsValid(DynMat))
			{
				bNeedsRebuild = true;
				break;
			}
		}
	}

	if (bNeedsRebuild)
	{
		TArray<UMaterialInterface*>& StoredOriginals = OriginalMaterials.FindOrAdd(HitComp);
		StoredOriginals.Reset();
		DynamicMaterials.FindOrAdd(HitComp).Reset();

		StoredOriginals.Reserve(NumMaterials);
		TArray<UMaterialInstanceDynamic*>& NewDynMats = DynamicMaterials.FindOrAdd(HitComp);
		NewDynMats.Reserve(NumMaterials);

		for (int32 Index = 0; Index < NumMaterials; ++Index)
		{
			UMaterialInterface* BaseMat = HitComp->GetMaterial(Index);
			StoredOriginals.Add(BaseMat);

			if (!BaseMat)
			{
				NewDynMats.Add(nullptr);
				continue;
			}

			UMaterialInstanceDynamic* DynMat = UMaterialInstanceDynamic::Create(BaseMat, this);
			NewDynMats.Add(DynMat);
			HitComp->SetMaterial(Index, DynMat);
		}

		DynMats = &NewDynMats;
	}

	static const FName LineStartParam(TEXT("LineStartWS"));
	static const FName LineEndParam(TEXT("LineEndWS"));
	static const FName CylinderRadiusParam(TEXT("CylinderRadius"));
	static const FName CylinderSoftnessParam(TEXT("CylinderSoftness"));
	static const FName ScreenCenterParam(TEXT("ScreenCenterUV"));
	static const FName ScreenRadiusParam(TEXT("ScreenRadius"));
	static const FName ScreenSoftnessParam(TEXT("ScreenSoftness"));
	static const FName ScreenAspectParam(TEXT("ScreenAspect"));
	static const FName HoleCenterParam(TEXT("HoleCenterWS"));
	static const FName HoleRadiusParam(TEXT("HoleRadius"));
	static const FName HoleSoftnessParam(TEXT("HoleSoftness"));

	if (DynMats)
	{
		for (UMaterialInstanceDynamic* DynMat : *DynMats)
		{
			if (!IsValid(DynMat))
			{
				continue;
			}

			DynMat->SetVectorParameterValue(LineStartParam, LineStart);
			DynMat->SetVectorParameterValue(LineEndParam, LineEnd);
			DynMat->SetScalarParameterValue(CylinderRadiusParam, ObstructionHoleRadius);
			DynMat->SetScalarParameterValue(CylinderSoftnessParam, ObstructionHoleSoftness);

			DynMat->SetVectorParameterValue(ScreenCenterParam, FVector(ScreenCenterUV, 0.f));
			DynMat->SetScalarParameterValue(ScreenRadiusParam, ScreenRadius);
			DynMat->SetScalarParameterValue(ScreenSoftnessParam, ScreenSoftness);
			DynMat->SetScalarParameterValue(ScreenAspectParam, ScreenAspect);

			DynMat->SetVectorParameterValue(HoleCenterParam, LineEnd);
			DynMat->SetScalarParameterValue(HoleRadiusParam, ObstructionHoleRadius);
			DynMat->SetScalarParameterValue(HoleSoftnessParam, ObstructionHoleSoftness);

			if (!ObstructionOpacityParamName.IsNone())
			{
				DynMat->SetScalarParameterValue(ObstructionOpacityParamName, Opacity);
			}
		}
	}
}

void UCameraObstructionComponent::ClearObstructionMaterial(UPrimitiveComponent* HitComp)
{
	if (!IsValid(HitComp))
	{
		return;
	}

	TArray<UMaterialInterface*>* StoredOriginals = OriginalMaterials.Find(HitComp);
	if (!StoredOriginals)
	{
		DynamicMaterials.Remove(HitComp);
		return;
	}

	const int32 NumMaterials = StoredOriginals->Num();
	for (int32 Index = 0; Index < NumMaterials; ++Index)
	{
		HitComp->SetMaterial(Index, (*StoredOriginals)[Index]);
	}

	OriginalMaterials.Remove(HitComp);
	DynamicMaterials.Remove(HitComp);
}
