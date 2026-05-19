#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UW_TempInteract.generated.h"

class UItemCommandBase;

DECLARE_DELEGATE(FOnExecuteCommand);

UCLASS()
class TARCOPY_API UUW_TempInteract : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetCommand(UItemCommandBase* InCommand);

private:
	UFUNCTION()
	void ExecuteInteract();

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TextInteract;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UButton> BtnInteract;

	UPROPERTY()
	TObjectPtr<UItemCommandBase> Command;

	FOnExecuteCommand OnExecuteCommand;
};
