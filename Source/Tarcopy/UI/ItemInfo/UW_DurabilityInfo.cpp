// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_DurabilityInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/DurabilityComponent.h"
#include "Item/Data/DurabilityData.h"
#include "Components/TextBlock.h"

void UUW_DurabilityInfo::BindItem(UItemInstance* InItem)
{
	CachedComponent = InItem->GetItemComponent<UDurabilityComponent>();
	CachedComponent->OnUpdatedItemComponent.AddUObject(this, &ThisClass::HandleUpdated);
	auto Data = CachedComponent->GetData();

	ConditionTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), CachedComponent->GetCondition(), Data->MaxCondition)));
}

void UUW_DurabilityInfo::HandleUpdated()
{
	ConditionTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), CachedComponent->GetCondition(), CachedComponent->GetData()->MaxCondition)));
}