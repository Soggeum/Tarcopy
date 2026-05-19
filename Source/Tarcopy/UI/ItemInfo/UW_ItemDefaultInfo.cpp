// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_ItemDefaultInfo.h"

#include "Item/ItemInstance.h"
#include "Item/Data/ItemData.h"
#include "Components/TextBlock.h"

void UUW_ItemDefaultInfo::BindItem(UItemInstance* InItem)
{
	auto Data = InItem->GetData();

	TextNameTxt->SetText(Data->TextName);

	auto Display = StaticEnum<EItemType>()->GetDisplayNameTextByValue((int64)Data->ItemType);
	ItemTypeTxt->SetText(Display);

	TextDescTxt->SetText(Data->TextDesc);
}