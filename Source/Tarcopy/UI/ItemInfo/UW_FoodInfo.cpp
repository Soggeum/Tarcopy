// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_FoodInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/FoodComponent.h"
#include "Item/Data/FoodData.h"
#include "Components/TextBlock.h"

void UUW_FoodInfo::BindItem(UItemInstance* InItem)
{
	CachedComponent = InItem->GetItemComponent<UFoodComponent>();
	CachedComponent->OnUpdatedItemComponent.AddUObject(this, &ThisClass::HandleUpdated);
	auto Data = CachedComponent->GetData();


	HungerTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->Hunger)));

	ThirstTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->Thirst)));

	RemainAmountTxt->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CachedComponent->GetRemainAmount() * 100.0f)));
}

void UUW_FoodInfo::HandleUpdated()
{
	RemainAmountTxt->SetText(FText::FromString(FString::Printf(TEXT("%.0f%%"), CachedComponent->GetRemainAmount() * 100.0f)));
}