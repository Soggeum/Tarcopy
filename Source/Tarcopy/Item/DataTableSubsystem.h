#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTableSubsystem.generated.h"

UENUM()
enum class EDataTableType
{
	ItemTable,
	MeleeWeaponTable,
	FirearmTable,
	FoodTable,
	ClothTable,
	FluidContainerTable,
	CraftTable,
	ContainerTable,
	DurabilityTable,
	MedicalTable,
	ItemSoundTable,
};

UCLASS(Blueprintable)
class TARCOPY_API UDataTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()	

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	const UDataTable* GetTable(EDataTableType TableType);

protected:
	UPROPERTY(EditDefaultsOnly)
	TMap<EDataTableType, TObjectPtr<UDataTable>> DataTables;
};
