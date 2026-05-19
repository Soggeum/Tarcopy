// Fill out your copyright notice in the Description page of Project Settings.


#include "Car/UI/TCCarWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Components/SizeBox.h"

void UTCCarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ImageFuel) return;

	UMaterialInterface* BaseMat =
		Cast<UMaterialInterface>(ImageFuel->GetBrush().GetResourceObject());

	if (!BaseMat)
	{
		UE_LOG(LogTemp, Error, TEXT("Fuel Image has no material"));
		return;
	}

	MIDFuel = UMaterialInstanceDynamic::Create(BaseMat, this);
	ImageFuel->SetBrushFromMaterial(MIDFuel);

	TWeakObjectPtr<UTCCarWidget> WeakThis(this);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpeedHandler,
			FTimerDelegate::CreateLambda([WeakThis]()
				{
					if (!WeakThis.IsValid())
						return;

					UTCCarWidget* Widget = WeakThis.Get();

					if (!Widget->TextSpeed)
						return;

					Widget->TextSpeed->SetText(
						FText::AsNumber(
							FMath::RoundToInt(Widget->CurrentSpeed)
						)
					);
				}),
			0.2f,
			true
		);
	}
}
void UTCCarWidget::UpdateSpeed(float InValue)
{
	if (!ImageSpeedNeedle) return;

	CurrentSpeed = FMath::Abs(InValue) * 0.036f;

	float Clamped = FMath::Clamp(CurrentSpeed, 0.f, MaxSpeed);
	float Ratio = Clamped / MaxSpeed;
	float Angle = FMath::Lerp(MinAngle, MaxAngle, Ratio);

	ImageSpeedNeedle->SetRenderTransformAngle(Angle);
}

void UTCCarWidget::UpdateRPM(float InValue)
{
	if (!ImageRPMNeedle) return;

	float ClampedRPM = FMath::Clamp(InValue, 0.f, MaxRPM);
	float Ratio = ClampedRPM / MaxRPM;
	float Angle = FMath::Lerp(MinAngle, MaxAngle, Ratio);

	ImageRPMNeedle->SetRenderTransformAngle(Angle);
}

void UTCCarWidget::UpdateFuel(float InValue)
{
	if (!ImageFuel) return;
	if (!MIDFuel) return;

	float Ratio = InValue / 100.f;
	Ratio = FMath::Clamp(Ratio, 0.f, 1.f);

	MIDFuel->SetScalarParameterValue(TEXT("FillAmount"), Ratio);
}

void UTCCarWidget::UpdateCarDamage(float Ratio)
{
	Ratio = FMath::Clamp(Ratio, 0.f, 1.f);

	FLinearColor Color = FLinearColor::LerpUsingHSV(
		FLinearColor::White,
		FLinearColor::Red,
		Ratio
	);

	Color.A = Ratio;

	/*ImageCarSection->SetColorAndOpacity(Color);*/
}
