// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_RootHUD.h"

void UUW_RootHUD::NativeConstruct()
{
    Super::NativeConstruct();
}

FReply UUW_RootHUD::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	OnGlobalMouseDown.Broadcast();

	return FReply::Unhandled();
}
