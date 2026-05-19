
#include "Car/Component/TCCarStatusComponent.h"
#include "TCCarStatusComponent.h"

UTCCarStatusComponent::UTCCarStatusComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UTCCarStatusComponent::UpdateSpeed(float NewSpeed)
{
	CurrentSpeed = FMath::Abs(NewSpeed) * 0.036f;
}

void UTCCarStatusComponent::UpdateGear(int32 NewGear)
{
}

void UTCCarStatusComponent::UpdateRpm(float NewRPM)
{
}


