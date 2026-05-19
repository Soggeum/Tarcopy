// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_InventoryCell.h"

#include "Components/Border.h"

void UUW_InventoryCell::SetPreviewColor(const FLinearColor& Color)
{
	if (!CellBG)
	{
		return;
	}

	if (!bSaved)
	{
		SavedBGColor = CellBG->GetBrushColor();
		bSaved = true;
	}

	CellBG->SetBrushColor(Color);
}

void UUW_InventoryCell::ClearPreview()
{
	if (!CellBG)
	{
		return;
	}

	if (bSaved)
	{
		CellBG->SetBrushColor(SavedBGColor);
	}
}