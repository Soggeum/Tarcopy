// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/UW_ItemInfo.h"

#include "Item/ItemInstance.h"
#include "Item/ItemComponent/ItemComponentPreset.h"
#include "Item/Data/ItemData.h"
#include "Item/ItemComponent/ItemComponentBase.h"
#include "Components/VerticalBox.h"
#include "UI/ItemInfo/ItemInfoConfig.h"
#include "UI/ItemInfo/UW_ItemInfoEntry.h"

void UUW_ItemInfo::BindItem(UItemInstance* Item)
{
	const FItemData* Data = Item->GetData();
	auto ItemComps = Item->GetItemComponents();

	APlayerController* PC = GetOwningPlayer();
	if(!PC)
	{
		return;
	}

	UUW_ItemInfoEntry* DefaultInfo = CreateWidget<UUW_ItemInfoEntry>(PC, DefaultInfoClass);
	DefaultInfo->BindItem(Item);
	VBox->AddChildToVerticalBox(DefaultInfo);
	
	if (IsValid(Data->ItemComponentPreset))
	{
		for (const auto& CompClass : Data->ItemComponentPreset->ItemComponentClasses)
		{
			for (auto Comp : ItemComps)
			{
				if (IsValid(Comp) && Comp->IsA(CompClass))
				{
					TSubclassOf<UUserWidget> WidgetClass;
					Config->GetInfo(CompClass, WidgetClass);
					UUW_ItemInfoEntry* W = CreateWidget<UUW_ItemInfoEntry>(PC, WidgetClass);
					W->BindItem(Item);
					VBox->AddChildToVerticalBox(W);
				}
			}
		}
	}
}