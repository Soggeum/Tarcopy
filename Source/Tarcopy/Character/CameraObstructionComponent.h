#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CameraObstructionComponent.generated.h"

class UCameraComponent;
class UCapsuleComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TARCOPY_API UCameraObstructionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCameraObstructionComponent();

	void SetCamera(UCameraComponent* InCamera);
	void SetCapsule(UCapsuleComponent* InCapsule);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateCameraObstructionFade();
	void ApplyObstructionMaterial(UPrimitiveComponent* HitComp, const FVector& LineStart, const FVector& LineEnd,
		const FVector2D& ScreenCenterUV, float ScreenRadius, float ScreenSoftness, float ScreenAspect, float Opacity);
	void ClearObstructionMaterial(UPrimitiveComponent* HitComp);

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ObstructionTraceInterval = 0.05f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float FadeHoldTime = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ObstructionHoleRadius = 120.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ObstructionHoleSoftness = 30.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ScreenRadiusPx = 140.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ScreenSoftnessPx = 20.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	bool bAffectComponentsInScreenRadius = true;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion", meta = (ClampMin = "0.1"))
	float ScreenCandidateRadiusScale = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	int32 MaxObstructionHits = 8;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	float ObstructionTraceStep = 2.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion")
	FName ObstructionOpacityParamName = TEXT("OpacityScalar");

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float FirstObstructionOpacity = 1.f;

	UPROPERTY(EditAnywhere, Category = "Vision|Occlusion", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float SubsequentObstructionOpacity = 0.5f;

	float TimeSinceLastObstructionTrace = 0.f;

	UPROPERTY()
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> Capsule;

	TMap<TWeakObjectPtr<UPrimitiveComponent>, float> FadeHoldUntil;
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TArray<UMaterialInterface*>> OriginalMaterials;
	TMap<TWeakObjectPtr<UPrimitiveComponent>, TArray<UMaterialInstanceDynamic*>> DynamicMaterials;
};
