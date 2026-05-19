#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

class UCharacterMovementComponent;
class UAnimationPreset;

UCLASS()
class TARCOPY_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    void SetAnimDataAsset(UAnimationPreset* InAnimDataAsset);

    // 애니메이션 변수들 (블루프린트에서 사용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<ACharacter> Character;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    TObjectPtr<UCharacterMovementComponent> MovementComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    float GroundSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    bool bShouldMove;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    bool bIsFalling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    bool bIsCrouch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EssentialMovementData")
    bool bIsAiming;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AnimDataAsset")
    TObjectPtr<UAnimationPreset> AnimDataAsset;
};
