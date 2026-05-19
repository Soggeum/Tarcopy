// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_InventoryCell.generated.h"

class UBorder;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_InventoryCell : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetPreviewColor(const FLinearColor& Color);

	void ClearPreview();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> CellBG;

	FLinearColor SavedBGColor;
	bool bSaved = false;
};
