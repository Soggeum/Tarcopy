// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Equipments/UW_PlayerEquipments.h"

#include "Item/EquipComponent.h"
#include "UI/Equipments/UW_SlotsByPart.h"

void UUW_PlayerEquipments::NativeDestruct()
{
    if (BoundEquipComp)
    {
        BoundEquipComp->OnChangedEquippedItems.RemoveAll(this);
    }
    Super::NativeDestruct();
}

void UUW_PlayerEquipments::BindEquipComponent(UEquipComponent* InEquipComp)
{
    if (!IsValid(InEquipComp))
    {
        return;
    }
    if (BoundEquipComp)
    {
        BoundEquipComp->OnChangedEquippedItems.RemoveAll(this);
    }

    BoundEquipComp = InEquipComp;

    if (BoundEquipComp)
    {
        BoundEquipComp->OnChangedEquippedItems.AddDynamic(this, &ThisClass::SetSlotByPart);
    }

    SetSlotByPart();
}

void UUW_PlayerEquipments::SetSlotByPart()
{
    if (!BoundEquipComp)
    {
        return;
    }

    if (HeadSlot)
    {
        HeadSlot->ClearItems();
    }
    if (BackSlot)
    {
        BackSlot->ClearItems();
    }
    if (TopSlot) 
    {
        TopSlot->ClearItems();
    }
    if (BottomSlot) 
    {
        BottomSlot->ClearItems();
    }
    if (ShoeSlot) 
    {
        ShoeSlot->ClearItems();
    }
    if (RightWristSlot)
    {
        RightWristSlot->ClearItems();
    }
    if (LeftWristSlot)
    {
        LeftWristSlot->ClearItems();
    }
    if (RightHandSlot) 
    {
        RightHandSlot->ClearItems();
    }
    if (LeftHandSlot) 
    {
        LeftHandSlot->ClearItems();
    }

    const TArray<FEquippedItemInfo>& ItemsInfo = BoundEquipComp->GetEquippedItemInfos();
    for (auto Item : ItemsInfo)
    {
        UUW_SlotsByPart* Group = GetGroupWidgetByLocation(Item.Location);
        if (Group)
        {
            Group->AddItem(Item);
        }
    }

    if (HeadSlot) 
    {
        HeadSlot->RefreshSlot();
    }
    if (BackSlot) 
    {
        BackSlot->RefreshSlot();
    }
    if (TopSlot) 
    {
        TopSlot->RefreshSlot();
    }
    if (BottomSlot)
    {
        BottomSlot->RefreshSlot();
    }
    if (ShoeSlot)
    {
        ShoeSlot->RefreshSlot();
    }
    if (RightWristSlot)
    {
        RightWristSlot->RefreshSlot();
    }
    if (LeftWristSlot)
    {
        LeftWristSlot->RefreshSlot();
    }
    if (RightHandSlot)
    {
        RightHandSlot->RefreshSlot();
    }
    if (LeftHandSlot)
    {
        LeftHandSlot->RefreshSlot();
    }
}

UUW_SlotsByPart* UUW_PlayerEquipments::GetGroupWidgetByLocation(EBodyLocation Loc) const
{
    switch (Loc)
    {
        // HeadSlot
    case EBodyLocation::Head:
    case EBodyLocation::Face:
    case EBodyLocation::Eyes:
    case EBodyLocation::Ear:
    case EBodyLocation::Neck:
    case EBodyLocation::Nose:
        return HeadSlot;

        // BackSlot
    case EBodyLocation::Back:
        return BackSlot;

        // TopSlot
    case EBodyLocation::TShirts:
    case EBodyLocation::ShortSleeveShirt:
    case EBodyLocation::Shirt:
    case EBodyLocation::Sweater:
    case EBodyLocation::TorsoExtra:
        return TopSlot;

        // BottomSlot
    case EBodyLocation::Bottoms:
        return BottomSlot;

        // ShoeSlot
    case EBodyLocation::Socks:
    case EBodyLocation::Shoes:
        return ShoeSlot;

    case EBodyLocation::Gloves:
    case EBodyLocation::RightWrist:
        return RightWristSlot;

    case EBodyLocation::LeftWrist:
        return LeftWristSlot;

    case EBodyLocation::RightHand:
        return RightHandSlot;
    case EBodyLocation::LeftHand:
        return LeftHandSlot;

    default:
        return nullptr;
    }
}