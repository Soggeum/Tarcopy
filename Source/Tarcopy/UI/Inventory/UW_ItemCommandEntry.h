// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ItemCommandEntry.generated.h"

class UButton;
class UTextBlock;
class UItemCommandBase;

DECLARE_MULTICAST_DELEGATE(FOnItemCommandEntryExecuted);

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ItemCommandEntry : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

public:
	void InitEntry(UItemCommandBase* InCommand);

private:
	UFUNCTION()
	void HandleClicked();

public:
	FOnItemCommandEntryExecuted OnExecuted;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt;

	UPROPERTY()
	TObjectPtr<UItemCommandBase> Command;
};
