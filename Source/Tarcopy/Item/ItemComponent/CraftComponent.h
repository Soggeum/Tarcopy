#pragma once

#include "CoreMinimal.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "CraftComponent.generated.h"

USTRUCT()
struct FItemSource
{
	GENERATED_BODY()

	class UInventoryData* OwnerInventory;
	UItemInstance* ItemInstance;

	FItemSource() {}
	FItemSource(UInventoryData* Inven, UItemInstance* InItem) : OwnerInventory(Inven), ItemInstance(InItem) {}
};

struct FCraftData;

UCLASS()
class TARCOPY_API UCraftComponent : public UItemComponentBase
{
	GENERATED_BODY()

public:
	virtual void SetOwnerItem(UItemInstance* InOwnerItem) override;
	virtual void GetCommands(TArray<TObjectPtr<class UItemCommandBase>>& OutCommands, const struct FItemCommandContext& Context) override;

protected:
	virtual void OnExecuteAction(AActor* InInstigator, const struct FItemNetworkContext& ActionContext) override;

public:
	void ExecuteCraft(AActor* InInstigator, const FName& CraftId);
};
