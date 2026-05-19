#include "Car/TCVehicleWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UTCVehicleWheelRear::UTCVehicleWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}
