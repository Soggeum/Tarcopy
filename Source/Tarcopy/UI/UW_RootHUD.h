// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "UW_RootHUD.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnRootHUDGlobalMouseDown);

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_RootHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	inline UCanvasPanel* GetRootCanvas() const { return RootCanvas.Get(); }

public:
	FOnRootHUDGlobalMouseDown OnGlobalMouseDown;

private:
	UPROPERTY(Meta = (BindWidget))
	TObjectPtr<UCanvasPanel> RootCanvas;
};
