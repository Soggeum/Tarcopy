// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_ContainerInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ContainerComponent.h"
#include "Item/Data/ContainerData.h"
#include "Components/TextBlock.h"

void UUW_ContainerInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetItemComponent<UContainerComponent>()->GetData();

	ContainerBoundTxt->SetText(FText::FromString(FString::Printf(TEXT("%d x %d"), Data->ContainerBound.X, Data->ContainerBound.Y)));

	WeightMultiplierTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), Data->WeightMultiplier)));
}