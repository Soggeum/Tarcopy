#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraObstructionFadeComponent.generated.h"

class UCameraComponent;
class UMeshComponent;

/**
 * Detects meshes between the camera and owner and fades them out to keep line of sight.
 * - Trace: SphereTrace from camera to owner at a configurable radius.
 * - Fade: Applies dynamic material instances and drives an opacity scalar parameter.
 * - State: Keeps track of obstructing actors per tick and restores materials when clear.
 */
UCLASS(ClassGroup = (Camera), meta = (BlueprintSpawnableComponent))
class TARCOPY_API UCameraObstructionFadeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraObstructionFadeComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Runs a sphere trace and updates obstructing actor sets.
	void UpdateObstructors();

	// Applies interpolation to mesh opacity and cleans up when fade-in completes.
	void ApplyFade(float DeltaTime);

	// Starts fading for all meshes on the actor toward TargetOpacity (0 = invisible, 1 = opaque).
	void StartFadeForActor(AActor* Actor, float TargetOpacity);

	// Ensures DMIs exist and updates scalar parameter on all slots of Mesh.
	void SetMeshOpacity(UMeshComponent* Mesh, float NewOpacity);

protected:
	UPROPERTY(EditAnywhere, Category = "Obstruction")
	float TraceRadius = 40.f;

	UPROPERTY(EditAnywhere, Category = "Obstruction")
	float TraceInterval = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Obstruction")
	float FadeOutTime = 0.2f;

	UPROPERTY(EditAnywhere, Category = "Obstruction")
	float FadeInTime = 0.2f;

	// Name of the scalar parameter in the material used to control opacity.
	UPROPERTY(EditAnywhere, Category = "Obstruction")
	FName OpacityParamName = TEXT("OpacityScalar");

	UPROPERTY(EditAnywhere, Category = "Obstruction")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	// Optional additional Z offset from owner location when tracing to reduce head clipping.
	UPROPERTY(EditAnywhere, Category = "Obstruction")
	float OwnerZOffset = 0.f;

private:
	struct FMeshFadeState
	{
		TArray<UMaterialInterface*> OriginalMaterials;
		float CurrentOpacity = 1.f;
		float TargetOpacity = 1.f;
	};

	// Actors currently blocking the view this frame.
	TSet<TWeakObjectPtr<AActor>> CurrentObstructors;

	// Actors that were blocking in the previous trace.
	TSet<TWeakObjectPtr<AActor>> PreviousObstructors;

	// Fade state per mesh.
	TMap<TWeakObjectPtr<UMeshComponent>, FMeshFadeState> MeshStates;

	// Cached pointers to speed up lookups.
	TWeakObjectPtr<UCameraComponent> CachedCamera;
	TWeakObjectPtr<class UCapsuleComponent> CachedCapsule;

	float TimeSinceLastTrace = 0.f;
};
