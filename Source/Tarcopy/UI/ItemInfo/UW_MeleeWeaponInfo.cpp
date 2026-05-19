// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_MeleeWeaponInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/MeleeWeaponComponent.h"
#include "Item/Data/MeleeWeaponData.h"
#include "Components/TextBlock.h"

void UUW_MeleeWeaponInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetItemComponent<UMeleeWeaponComponent>()->GetData();

	auto Display = StaticEnum<EBodyLocation>()->GetDisplayNameTextByValue((int64)Data->BodyLocation);;
	BodyLocationTxt->SetText(Display);

	DamageTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f ~ %.1f"), Data->MinDamage, Data->MaxDamage)));

	RangeTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f ~ %.1f"), Data->MinRange, Data->MaxRange)));

	AttackSpeedTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->AttackSpeed)));
}