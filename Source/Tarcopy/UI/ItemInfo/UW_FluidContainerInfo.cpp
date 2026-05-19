// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_FluidContainerInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/FluidContainerComponent.h"
#include "Item/Data/FluidContainerData.h"
#include "Components/TextBlock.h"

void UUW_FluidContainerInfo::BindItem(UItemInstance* InItem)
{
	ChachedComponent = InItem->GetItemComponent<UFluidContainerComponent>();
	auto Data = ChachedComponent->GetData();

	CapacityTxt->SetText(FText::FromString(FString::Printf(TEXT("%.1f / %.1f"), ChachedComponent->GetAmount(), Data->Capacity)));
}