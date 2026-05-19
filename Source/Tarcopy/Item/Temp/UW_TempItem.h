#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TempItem.generated.h"

class UItemInstance;

UCLASS()
class TARCOPY_API UUW_TempItem : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

public:
	void SetItem(UItemInstance* InItem);
	// 원래는 아이템 상호작용 리스트 끄고 전체 인벤토리 정보 갱신해야 함 (테스트용 임시)
	void UpdateTempItem();

	//
	void SetHunger(float CurrentValue, float MaxValue);
	void SetThirst(float CurrentValue, float MaxValue);
	
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextItemId;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUW_TempInteract> InteractUIClass;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UPanelWidget> PanelInteract;

	//
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextHunger;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextThirst;

	//

	UPROPERTY()
	TWeakObjectPtr<UItemInstance> Item;
};
