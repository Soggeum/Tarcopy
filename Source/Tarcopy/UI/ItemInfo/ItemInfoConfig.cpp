// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ItemInfo/ItemInfoConfig.h"

bool UItemInfoConfig::GetInfo(TSubclassOf<UItemComponentBase> Type, TSubclassOf<UUserWidget>& OutInfo) const
{
    if (const auto* Found = InfoConfig.Find(Type))
    {
        OutInfo = *Found;
        return true;
    }
    return false;
}