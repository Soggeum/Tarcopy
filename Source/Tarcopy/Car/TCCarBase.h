// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Car/Data/CarCommand.h"
#include "Character/ActivateInterface.h"
#include "TCCarBase.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
class USpotLightComponent;
class UTCCarCombatComponent;
class UTCCarWidget;
class UUISubsystem;
class UTCCarActivate;
class USceneComponent;
struct FInputActionValue;
class UVisionComponent;

UCLASS(abstract)
class ATCCarBase : public AWheeledVehiclePawn, public IActivateInterface
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UTCCarCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* SceneComponent;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Light;

	UPROPERTY(EditDefaultsOnly)
	USpotLightComponent* HeadLight_R;

	UPROPERTY(EditDefaultsOnly)
	USpotLightComponent* HeadLight_L;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVisionComponent> VisionComponent;

	UPROPERTY(EditDefaultsOnly)
	UAudioComponent* EngineAudioComp;

	
 
	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

protected:

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SteeringAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BrakeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* HandbrakeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LightAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InterAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* WheelAction;

public:
	ATCCarBase();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;

	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float Delta) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void UnPossessed() override;

	virtual float TakeDamage(
		float DamageAmount,
		FDamageEvent const& DamageEvent,
		AController* EventInstigator,
		AActor* DamageCauser) override;
protected:

	void Steering(const FInputActionValue& Value);

	void Throttle(const FInputActionValue& Value);

	void Brake(const FInputActionValue& Value);

	void StartBrake(const FInputActionValue& Value);
	void StopBrake(const FInputActionValue& Value);

	void StartHandbrake(const FInputActionValue& Value);
	void StopHandbrake(const FInputActionValue& Value);

	void ToggleLight(const FInputActionValue& Value);

	void StartWheel(const FInputActionValue& Value);

	void StartInterAction(const FInputActionValue& Value);

public:

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoSteering(float SteeringValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoThrottle(float ThrottleValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoBrake(float BrakeValue);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoBrakeStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoBrakeStop();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoHandbrakeStart();

	UFUNCTION(BlueprintCallable, Category = "Input")
	void DoHandbrakeStop();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Input")
	void ServerRPCDoHandLight();

protected:

	UFUNCTION(BlueprintImplementableEvent, Category = "Vehicle")
	void BrakeLights(bool bBraking);

	UFUNCTION(Server, Reliable)
	void ServerRPCUpdateFuel(float InValue);

	UFUNCTION()
	void OnRep_UpdateGas();

	UFUNCTION()
	void OnRep_bLightOn();

public:
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_UpdateGas)
	float CurrentFuel;
	
	UPROPERTY()
	float MaxFuel;

	UPROPERTY(ReplicatedUsing = OnRep_bLightOn)
	uint8 bLightOn : 1;

	FTimerHandle GasHandler;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Gas");
	float MoveFactor;

	UPROPERTY(Replicated)
	uint8 bCanMove : 1;

public:
	FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return Camera; }
	FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

	UPROPERTY()
	TObjectPtr<UTCCarWidget> CarWidgetInstance;

	UPROPERTY()
	TObjectPtr<UUISubsystem> UISubsystem;

#pragma region Possess
public:

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_Controller() override;

	virtual void Activate(AActor* InInstigator) override;

	UFUNCTION()
	void ShowCharacter(APawn* InPawn, APlayerController* InPC);

	UFUNCTION(NetMulticast, Reliable)

	void MulticastShowCharacter(APawn* InPawn, const FVector& OutLocation,const FRotator& OutRotation);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastHideCharacter(APawn* InPawn);
	UFUNCTION()
	bool FindDismountLocation(APawn* InPawn, FVector& OutLocation) const;

#pragma endregion

#pragma region Damage

	UPROPERTY(Replicated)
	float SteeringFactor = 1.f;

	UPROPERTY(Replicated)
	float ThrottleFactor = 1.f;
	
	void DisableWheel(UPrimitiveComponent* DestroyComponent);

#pragma region endregion

#pragma region Activate
	UFUNCTION()
	void ExecuteCommand(ECarCommand Command, APawn* InPawn, APlayerController* InPC);

	UFUNCTION(BlueprintCallable)
	TArray<ECarCommand> GetAvailableCommands();

	UFUNCTION()
	void ShowInterActionUI(APlayerController* InPC);

	UFUNCTION()
	void SitByPassenger(APawn* InPawn, APlayerController* InPC);

	UFUNCTION()
	void ExitVehicle(APawn* InPawn, APlayerController* InPC);

	UPROPERTY()
	TObjectPtr<UTCCarActivate> UI;

	UPROPERTY(ReplicatedUsing = OnRep_Passengers)
	TArray<APawn*> Passengers;

	UFUNCTION()
	void OnRep_Passengers();

	UFUNCTION()
	void AddPassenger(APawn* InPawn,bool IsDriver);

	UPROPERTY(Replicated)
	bool bCanRide;

	UFUNCTION()
	void SitByDriver(APawn* InPawn, APlayerController* InPC);

	UPROPERTY(Replicated)
	TObjectPtr<APawn> DriverPawn;

	UPROPERTY(Replicated)
	uint8 IsDriverPawn : 1;
#pragma region endregion

#pragma region Audio

	FTimerHandle SoundHandler;

	UPROPERTY(ReplicatedUsing = OnRep_bEngineOn)
	uint8 bEngineOn : 1;

	UFUNCTION()
	void OnRep_bEngineOn();

	
};
