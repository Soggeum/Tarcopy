// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_MedicalInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/MedicalComponent.h"
#include "Item/Data/MedicalData.h"
#include "Components/TextBlock.h"

void UUW_MedicalInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetItemComponent<UMedicalComponent>()->GetData();

	RestoreAmountTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->RestoreAmount)));
}