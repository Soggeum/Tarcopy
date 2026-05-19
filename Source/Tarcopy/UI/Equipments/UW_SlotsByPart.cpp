// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipments/UW_SlotsByPart.h"

#include "Components/NamedSlot.h"
#include "Components/ScrollBox.h"
#include "Item/EquipComponent.h"
#include "UI/Equipments/UW_EquipmentSlot.h"

void UUW_SlotsByPart::AddItem(FEquippedItemInfo Info)
{
    Items.Add(Info);
}

void UUW_SlotsByPart::RefreshSlot()
{
    if (MainSlot)
    {
        if (UWidget* Content = MainSlot->GetContent())
        {
            Content->RemoveFromParent();
        }

        MainSlot->SetContent(nullptr);
    }

    if (SubSlots)
    {
        SubSlots->ClearChildren();
    }

    if (Items.Num() == 0)
    {
        return;
    }
    if (!EquipmentSlotClass)
    {
        return;
    }

    Items.Sort([](const FEquippedItemInfo& A, const FEquippedItemInfo& B)
        {
            return (uint32)A.Location < (uint32)B.Location;
        });

    Items.RemoveAll([](const FEquippedItemInfo& Info)
        {
            return Info.Item == nullptr;
        });
    if (Items.Num() == 0)
    {
        return;
    }

    UUW_EquipmentSlot* MainWidget = CreateWidget<UUW_EquipmentSlot>(GetWorld(), EquipmentSlotClass);
    if (MainWidget)
    {
        MainWidget->SetItem(Items[0].Item);
        MainWidget->SetSize(MainSize);

        if (MainSlot)
        {
            MainSlot->SetContent(MainWidget);
        }
    }

    if (SubSlots)
    {
        for (int32 i = 1; i < Items.Num(); ++i)
        {
            UUW_EquipmentSlot* SubWidget = CreateWidget<UUW_EquipmentSlot>(GetWorld(), EquipmentSlotClass);

            SubWidget->SetItem(Items[i].Item);
            SubWidget->SetSize(SubSize);

            SubSlots->AddChild(SubWidget);
        }
        if (SubSlots->GetChildrenCount())
        {
            SubSlots->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            SubSlots->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}