// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_ItemCommandMenu.generated.h"

class UItemInstance;
class UItemCommandBase;
class UVerticalBox;
class UUW_ItemCommandEntry;

/**
 * 
 */
UCLASS()
class TARCOPY_API UUW_ItemCommandMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	void InitMenu(UItemInstance* InItem);

private:
	void RebuildEntries();

	UFUNCTION()
	void HandleEntryExecuted();

	bool bInitialized = false;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> PanelEntries;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUW_ItemCommandEntry> EntryClass;

	UPROPERTY()
	TWeakObjectPtr<UItemInstance> Item;

	UPROPERTY()
	TArray<TObjectPtr<UItemCommandBase>> Commands;
};
