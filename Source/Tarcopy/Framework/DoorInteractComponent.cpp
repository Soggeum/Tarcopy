// Minimal component to toggle yaw on tagged StaticMeshActors without dedicated BP actors.

#include "Framework/DoorInteractComponent.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Character/MyCharacter.h"
#include "Framework/DoorTagUtils.h"
#include "Kismet/GameplayStatics.h"

UDoorInteractComponent::UDoorInteractComponent()
	: OpenYawOffset(90.f)
	, ClosedYawOffset(0.f)
	, bStartOpen(false)
	, bIsOpen(false)
	, InitialLocation(FVector::ZeroVector)
	, InitialRotation(FRotator::ZeroRotator)
	, InitialYaw(0.f)
	, bInitialized(false)
	, VisualizerOverlapCount(0)
	, bInteractionVisualizerInitialized(false)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDoorInteractComponent::OnRegister()
{
	Super::OnRegister();

	if (!InteractionVisualizer)
	{
		if (AActor* Owner = GetOwner())
		{
			InteractionVisualizer = NewObject<UBoxComponent>(Owner, TEXT("DoorInteractVisualizer"));
			if (InteractionVisualizer)
			{
				// ?��? ?�랜?�폼???�용??문의 ?�전???�향??받�? ?�고 초기 ?�치/방향???��?.
				InteractionVisualizer->SetUsingAbsoluteLocation(true);
				InteractionVisualizer->SetUsingAbsoluteRotation(true);
				InteractionVisualizer->SetUsingAbsoluteScale(true);

				bInteractionVisualizerInitialized = false;
				UpdateInteractionBoxFromOwner();
				InteractionVisualizer->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				InteractionVisualizer->SetGenerateOverlapEvents(true);
				InteractionVisualizer->SetCollisionResponseToAllChannels(ECR_Ignore);
				InteractionVisualizer->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
				InteractionVisualizer->SetCanEverAffectNavigation(false);
				InteractionVisualizer->SetVisibility(false, true);
				InteractionVisualizer->SetHiddenInGame(true);
				InteractionVisualizer->SetCastShadow(false);
				InteractionVisualizer->bIsEditorOnly = false;
				InteractionVisualizer->ShapeColor = FColor::Red;
				InteractionVisualizer->SetupAttachment(Owner->GetRootComponent());
				InteractionVisualizer->OnComponentBeginOverlap.AddDynamic(this, &UDoorInteractComponent::OnVisualizerBeginOverlap);
				InteractionVisualizer->OnComponentEndOverlap.AddDynamic(this, &UDoorInteractComponent::OnVisualizerEndOverlap);
				InteractionVisualizer->RegisterComponent();
			}
		}
	}
	else
	{
		UpdateInteractionBoxFromOwner();
		UpdateDoorOutline(false);
		InteractionVisualizer->SetVisibility(false, true);
		InteractionVisualizer->SetHiddenInGame(true);
	}
}

void UDoorInteractComponent::OnUnregister()
{
	if (InteractionVisualizer)
	{
		InteractionVisualizer->DestroyComponent();
		InteractionVisualizer = nullptr;
		bInteractionVisualizerInitialized = false;
	}

	Super::OnUnregister();
}

void UDoorInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	if (AActor* Owner = GetOwner())
	{
		CacheDoorMeshes();
		EnsureMovableMesh();

		static const FName DoorSlideTag(TEXT("DoorSlide"));
		if (Owner->ActorHasTag(DoorSlideTag))
		{
			MotionType = EDoorMotionType::Slide;
		}

		UpdateInteractionBoxFromOwner();
		// ?�호?�용 ?�역(Visualizer) ?�시??비활?�화?�니??
		// UpdateVisualizerColor(false);

		InitialLocation = Owner->GetActorLocation();
		InitialRotation = Owner->GetActorRotation();
		InitialYaw = InitialRotation.Yaw;
		RefreshAutoSlideOffsetsFromBounds();
		bIsOpen = bStartOpen;
		bInitialized = true;
		ApplyDoorState();
	}
}

void UDoorInteractComponent::ToggleDoor()
{
	ToggleDoorInternal(true);
}

void UDoorInteractComponent::Activate(AActor* InInstigator)
{
	ToggleDoorInternal(true);
}

void UDoorInteractComponent::ApplyDoorStateFromServer(bool bOpen)
{
	InitializeIfNeeded();
	bIsOpen = bOpen;
	ApplyDoorState();
}

void UDoorInteractComponent::ApplyDoorState()
{
	if (DoorMeshComponents.Num() > 0 && DoorMeshComponents.Num() == DoorMeshInitialRelativeTransforms.Num())
	{
		for (int32 Index = 0; Index < DoorMeshComponents.Num(); ++Index)
		{
			UStaticMeshComponent* MeshComp = DoorMeshComponents[Index].Get();
			if (!IsValid(MeshComp))
			{
				continue;
			}

			const FTransform& InitialRel = DoorMeshInitialRelativeTransforms[Index];
			if (MotionType == EDoorMotionType::Slide)
			{
				const FVector LocalOffset = bIsOpen ? OpenLocalOffset : ClosedLocalOffset;
				MeshComp->SetRelativeLocation(InitialRel.GetLocation() + LocalOffset);
			}
			else
			{
				FRotator Rot = InitialRel.Rotator();
				Rot.Yaw = Rot.Yaw + (bIsOpen ? OpenYawOffset : ClosedYawOffset);
				MeshComp->SetRelativeRotation(Rot);
			}
		}

		return;
	}

	if (AActor* Owner = GetOwner())
	{
		if (MotionType == EDoorMotionType::Slide)
		{
			const FVector LocalOffset = bIsOpen ? OpenLocalOffset : ClosedLocalOffset;
			const FVector WorldOffset = InitialRotation.RotateVector(LocalOffset);
			Owner->SetActorLocation(InitialLocation + WorldOffset);
		}
		else
		{
			FRotator Rot = InitialRotation;
			Rot.Yaw = InitialYaw + (bIsOpen ? OpenYawOffset : ClosedYawOffset);
			Owner->SetActorRotation(Rot);
		}
	}
}

void UDoorInteractComponent::EnsureMovableMesh() const
{
	if (DoorMeshComponents.Num() > 0)
	{
		for (const TWeakObjectPtr<UStaticMeshComponent>& MeshPtr : DoorMeshComponents)
		{
			if (UStaticMeshComponent* Mesh = MeshPtr.Get())
			{
				if (Mesh->Mobility != EComponentMobility::Movable)
				{
					Mesh->SetMobility(EComponentMobility::Movable);
				}
			}
		}

		return;
	}

	if (const AActor* Owner = GetOwner())
	{
		if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Owner->GetRootComponent()))
		{
			if (Mesh->Mobility != EComponentMobility::Movable)
			{
				Mesh->SetMobility(EComponentMobility::Movable);
			}
		}
	}
}

FName UDoorInteractComponent::ResolveDoorGroupTag() const
{
	if (DoorGroupTag != NAME_None)
	{
		return DoorGroupTag;
	}

	static const FString DoorGroupPrefix(TEXT("DoorGroup_"));
	if (const AActor* Owner = GetOwner())
	{
		for (const FName& Tag : Owner->Tags)
		{
			if (Tag.ToString().StartsWith(DoorGroupPrefix))
			{
				return Tag;
			}
		}
	}

	return NAME_None;
}

void UDoorInteractComponent::InitializeIfNeeded()
{
	if (bInitialized)
	{
		return;
	}

	if (AActor* Owner = GetOwner())
	{
		CacheDoorMeshes();
		EnsureMovableMesh();

		static const FName DoorSlideTag(TEXT("DoorSlide"));
		if (Owner->ActorHasTag(DoorSlideTag))
		{
			MotionType = EDoorMotionType::Slide;
		}

		InitialLocation = Owner->GetActorLocation();
		InitialRotation = Owner->GetActorRotation();
		InitialYaw = InitialRotation.Yaw;
		RefreshAutoSlideOffsetsFromBounds();
		bIsOpen = bStartOpen;
		bInitialized = true;
	}
}

void UDoorInteractComponent::CacheDoorMeshes()
{
	DoorMeshComponents.Reset();
	DoorMeshInitialRelativeTransforms.Reset();

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	TInlineComponentArray<UStaticMeshComponent*> MeshComponents(Owner);
	for (UStaticMeshComponent* MeshComp : MeshComponents)
	{
		if (IsValid(MeshComp) && MeshComp->ComponentHasTag(GetDoorTagName()))
		{
			DoorMeshComponents.Add(MeshComp);
			DoorMeshInitialRelativeTransforms.Add(MeshComp->GetRelativeTransform());
		}
	}
}

void UDoorInteractComponent::RefreshAutoSlideOffsetsFromBounds()
{
	if (MotionType != EDoorMotionType::Slide || !bAutoSlideFromBounds)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	const FTransform OwnerTransform = Owner->GetActorTransform();
	FBox LocalBounds(ForceInit);

	if (DoorMeshComponents.Num() > 0)
	{
		if (const UStaticMeshComponent* MeshComp = DoorMeshComponents[0].Get())
		{
			LocalBounds = MeshComp->Bounds.GetBox().TransformBy(OwnerTransform.Inverse());
		}
	}

	if (!LocalBounds.IsValid)
	{
		LocalBounds = Owner->GetComponentsBoundingBox(true).TransformBy(OwnerTransform.Inverse());
	}

	const FVector LocalExtent = LocalBounds.GetExtent(); // owner-local half size

	// FVector(UE::Math::TVector<double>) does not provide GetMinAxis/GetMaxAxis in UE5,
	// so we compute the axis manually.
	const int32 ThicknessAxis =
		(LocalExtent.X <= LocalExtent.Y && LocalExtent.X <= LocalExtent.Z) ? 0 :
		((LocalExtent.Y <= LocalExtent.Z) ? 1 : 2);

	// Choose slide axis from the two non-thickness axes, preferring a horizontal axis (X/Y) over vertical (Z).
	TArray<int32> CandidateAxes;
	CandidateAxes.Reserve(2);
	for (int32 Axis = 0; Axis < 3; ++Axis)
	{
		if (Axis != ThicknessAxis)
		{
			CandidateAxes.Add(Axis);
		}
	}

	int32 SlideAxis = CandidateAxes[0];
	if (CandidateAxes.Contains(2) && SlideAxis == 2)
	{
		SlideAxis = CandidateAxes[1];
	}
	else if (!CandidateAxes.Contains(2) || CandidateAxes[1] != 2)
	{
		SlideAxis = (LocalExtent[CandidateAxes[1]] > LocalExtent[CandidateAxes[0]]) ? CandidateAxes[1] : CandidateAxes[0];
	}

	FVector LocalDir = FVector::ZeroVector;
	LocalDir[SlideAxis] = 1.0f;

	static const FName DoorSlideReverseTag(TEXT("DoorSlideReverse"));
	if (Owner->ActorHasTag(DoorSlideReverseTag))
	{
		LocalDir *= -1.0f;
	}

	const float SlideDistance = LocalExtent[SlideAxis] * 2.0f * AutoSlideDistanceMultiplier;

	ClosedLocalOffset = FVector::ZeroVector;
	OpenLocalOffset = LocalDir * SlideDistance;
}

void UDoorInteractComponent::SetDoorOpenInternal(bool bOpen)
{
	InitializeIfNeeded();
	bIsOpen = bOpen;
	ApplyDoorState();
}

void UDoorInteractComponent::ToggleDoorInternal(bool bPropagateToGroup)
{
	InitializeIfNeeded();

	const bool bNewOpen = !bIsOpen;

	if (!bPropagateToGroup)
	{
		SetDoorOpenInternal(bNewOpen);
		return;
	}

	const FName GroupTag = ResolveDoorGroupTag();
	if (GroupTag == NAME_None)
	{
		SetDoorOpenInternal(bNewOpen);
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		SetDoorOpenInternal(bNewOpen);
		return;
	}

	TArray<AActor*> GroupActors;
	UGameplayStatics::GetAllActorsWithTag(World, GroupTag, GroupActors);
	if (GroupActors.IsEmpty())
	{
		SetDoorOpenInternal(bNewOpen);
		return;
	}

	for (AActor* DoorActor : GroupActors)
	{
		if (!ActorHasDoorTagOrDoorMesh(DoorActor))
		{
			continue;
		}

		UDoorInteractComponent* DoorComp = DoorActor->FindComponentByClass<UDoorInteractComponent>();
		if (!DoorComp)
		{
			DoorComp = NewObject<UDoorInteractComponent>(DoorActor);
			if (!IsValid(DoorComp))
			{
				continue;
			}
			DoorComp->RegisterComponent();
		}

		DoorComp->SetDoorOpenInternal(bNewOpen);
	}
}

void UDoorInteractComponent::UpdateInteractionBoxFromOwner()
{
	if (AActor* Owner = GetOwner())
	{
		if (bInteractionVisualizerInitialized)
		{
			return;
		}

		const FTransform OwnerTransform = Owner->GetActorTransform();
		const FBox LocalBounds = Owner->GetComponentsBoundingBox(true).TransformBy(OwnerTransform.Inverse());
		const FVector LocalExtent = LocalBounds.GetExtent(); // owner-local half size
		const FVector LocalCenter = LocalBounds.GetCenter(); // owner-local center
		const FRotator WorldRotation = Owner->GetActorRotation(); // capture initial facing

		if (InteractionVisualizer)
		{
			// ??X/Y)?� 1.1배로 ?�간 ?�게, ?�이(Z)???�일?�게 ?��?.
			const FVector AdjustedExtent(LocalExtent.X * 1.1f, LocalExtent.Y * 1.1f, LocalExtent.Z);
			InteractionVisualizer->SetBoxExtent(AdjustedExtent);
			InteractionVisualizer->SetWorldLocation(OwnerTransform.TransformPosition(LocalCenter));
			InteractionVisualizer->SetWorldRotation(WorldRotation);
			bInteractionVisualizerInitialized = true;
		}
	}
}

void UDoorInteractComponent::UpdateVisualizerColor(bool bHasCharacterInside)
{
	if (InteractionVisualizer)
	{
		InteractionVisualizer->ShapeColor = bHasCharacterInside ? FColor::Green : FColor::Red;
		InteractionVisualizer->MarkRenderStateDirty();
	}
}

void UDoorInteractComponent::UpdateDoorOutline(bool bEnable)
{
	if (!bEnableProximityOutline)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	Owner->GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* Comp : PrimitiveComponents)
	{
		if (!IsValid(Comp) || Comp == InteractionVisualizer)
		{
			continue;
		}

		Comp->SetRenderCustomDepth(bEnable);
		if (bEnable)
		{
			Comp->SetCustomDepthStencilValue(OutlineStencilValue);
		}
	}
}

void UDoorInteractComponent::OnVisualizerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(OtherActor))
	{
		if (!MyChar->IsPlayerControlled())
		{
			return;
		}

		VisualizerOverlapCount++;
		UpdateVisualizerColor(true);
		UpdateDoorOutline(true);

		MyChar->AddInteractableDoor(GetOwner());
	}
}

void UDoorInteractComponent::OnVisualizerEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AMyCharacter* MyChar = Cast<AMyCharacter>(OtherActor))
	{
		if (!MyChar->IsPlayerControlled())
		{
			return;
		}

		VisualizerOverlapCount = FMath::Max(0, VisualizerOverlapCount - 1);
		UpdateVisualizerColor(VisualizerOverlapCount > 0);
		UpdateDoorOutline(VisualizerOverlapCount > 0);

		MyChar->RemoveInteractableDoor(GetOwner());
	}
}
