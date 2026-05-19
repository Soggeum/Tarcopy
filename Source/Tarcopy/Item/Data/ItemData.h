#pragma once

#include "CoreMinimal.h"
#include "Item/ItemEnums.h"
#include "ItemData.generated.h"

USTRUCT(BlueprintType)
struct TARCOPY_API FItemData : public FTableRowBase
{
public:
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemId;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EItemType ItemType = EItemType::Ammo;										// UI 출력용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TextName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText TextDesc;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> ItemIcon;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMesh> DefaultMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FIntPoint InventoryBound = FIntPoint(2, 1);
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 0.0f;
	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Bitmask, BitmaskEnum = "/Script/Tarcopy.EItemCategory"))
	int32 ItemCategory = 0;*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ToolEfficiency = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<class UItemComponentPreset> ItemComponentPreset = nullptr;
};
