// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_ClothInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ClothingComponent.h"
#include "Item/Data/ClothData.h"
#include "Components/TextBlock.h"

void UUW_ClothInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetItemComponent<UClothingComponent>()->GetData();

	auto Display = StaticEnum<EBodyLocation>()->GetDisplayNameTextByValue((int64)Data->BodyLocation);;
	BodyLocationTxt->SetText(Display);

	DamageReduceTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->DamageReduce)));
}