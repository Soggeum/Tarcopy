#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CraftSubsystem.generated.h"

USTRUCT()
struct TARCOPY_API FCraftRecipe
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FDataTableRowHandle> CraftDataHandles;
};

UCLASS()
class TARCOPY_API UCraftSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	void LoadRecipes();
	const FCraftRecipe* GetCraftRecipe(const FName& IngredientId);

	void ExecuteCraft(AActor* InInstiagor, const FName& CraftId);

protected:
	UPROPERTY()
	uint8 bIsLoaded : 1 = false;

	UPROPERTY()
	TMap<FName, FCraftRecipe> CraftRecipes;
};
