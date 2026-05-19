// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_FirearmInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/FirearmComponent.h"
#include "Item/Data/FirearmData.h"
#include "Components/TextBlock.h"

void UUW_FirearmInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetItemComponent<UFirearmComponent>()->GetData();

	auto Display = StaticEnum<EBodyLocation>()->GetDisplayNameTextByValue((int64)Data->BodyLocation);;
	BodyLocationTxt->SetText(Display);

	DamageTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f ~ %.1f"), Data->MinDamage, Data->MaxDamage)));

	RangeTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f ~ %.1f"), Data->MinRange, Data->MaxRange)));

	AccurancyTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->Accurancy)));
}