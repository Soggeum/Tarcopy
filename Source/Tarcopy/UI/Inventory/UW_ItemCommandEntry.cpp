// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Inventory/UW_ItemCommandEntry.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "Kismet/KismetSystemLibrary.h"

void UUW_ItemCommandEntry::NativeDestruct()
{
	if (Btn)
	{
		Btn->OnClicked.RemoveAll(this);
	}
	Super::NativeDestruct();
}

void UUW_ItemCommandEntry::InitEntry(UItemCommandBase* InCommand)
{
	Command = InCommand;
	if (!IsValid(Command) || !Btn || !Txt)
	{
		return;
	}

	Txt->SetText(Command->TextDisplay);
	Btn->SetIsEnabled(Command->bExecutable);

	Btn->OnClicked.RemoveAll(this);
	Btn->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);

	UE_LOG(LogTemp, Warning, TEXT("[CmdEntry] InitEntry this=%p Btn=%s Txt=%s Cmd=%s Exec=%d"),
		this, *GetNameSafe(Btn), *GetNameSafe(Txt), *GetNameSafe(Command), Command ? (int32)Command->bExecutable : -1);
}

void UUW_ItemCommandEntry::HandleClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("UUW_ItemCommandEntry : HandleClicked"));
	if (!IsValid(Command))
	{
		return;
	}

	if (!Command->bExecutable)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), TEXT("Command is not executable"));
		return;
	}

	FItemCommandContext Ctx;
	Ctx.InstigatorController = GetOwningPlayer();
	if (Ctx.InstigatorController.IsValid())
	{
		Ctx.Instigator = Cast<AActor>(Ctx.InstigatorController->GetPawn());
	}

	Command->Execute(Ctx);
	OnExecuted.Broadcast();
}