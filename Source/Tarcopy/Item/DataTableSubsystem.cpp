#include "Item/DataTableSubsystem.h"

void UDataTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

bool UDataTableSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return this->GetClass()->IsInBlueprint() == true && Super::ShouldCreateSubsystem(Outer) == true;
}

const UDataTable* UDataTableSubsystem::GetTable(EDataTableType TableType)
{
	if (const auto* PtrTable = DataTables.Find(TableType))
	{
		return *PtrTable;
	}
	return nullptr;
}
