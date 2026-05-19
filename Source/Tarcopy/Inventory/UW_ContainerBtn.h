// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ContainerBtn.generated.h"

class UButton;
class UImage;
class UInventoryData;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnClickedWithInventory, UInventoryData* /*Inventory*/);

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ContainerBtn : public UUserWidget
{
	GENERATED_BODY()
	
public:


protected:
    virtual void NativeConstruct() override;

public:
    void BindInventory(UInventoryData* InInventory, FName Type);

    void RefreshVisual(FName Type);

private:
    UFUNCTION()
    void HandleClicked();

public:
    FOnClickedWithInventory OnClickedWithInventory;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> ContainerBtn;

private:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ContainerImg;

    UPROPERTY()
    FText DisplayName;

    UPROPERTY()
    TObjectPtr<UInventoryData> Inventory;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Icons")
    TObjectPtr<UTexture2D> Icon_Box;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Icons")
    TObjectPtr<UTexture2D> Icon_Bag;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Icons")
    TObjectPtr<UTexture2D> Icon_Zombie;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Icons")
    TObjectPtr<UTexture2D> Icon_Ground;
};
