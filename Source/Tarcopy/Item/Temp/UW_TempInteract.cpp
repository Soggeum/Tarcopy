#include "Item/Temp/UW_TempInteract.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Item/ItemCommand/ItemCommandBase.h"
#include "Kismet/KismetSystemLibrary.h"

void UUW_TempInteract::SetCommand(UItemCommandBase* InCommand)
{
	if (IsValid(InCommand) == false)
		return;

	Command = InCommand;

	TextInteract->SetText(Command->TextDisplay);
	BtnInteract->SetIsEnabled(Command->bExecutable);
	BtnInteract->OnClicked.AddDynamic(this, &ThisClass::ExecuteInteract);
}

void UUW_TempInteract::ExecuteInteract()
{
	if (IsValid(Command) == false)
		return;

	if (Command->bExecutable == false)
	{
		FString CommandString = Command->TextDisplay.ToString();
		UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("%s is not executable"), *CommandString));
		return;
	}

	FItemCommandContext CommandContext;
	CommandContext.InstigatorController = GetOwningPlayer();
	if (CommandContext.InstigatorController.IsValid() == true)
	{
		CommandContext.Instigator = Cast<AActor>(CommandContext.InstigatorController->GetPawn());
	}

	Command->Execute(CommandContext);

	if (OnExecuteCommand.IsBound())
	{
		OnExecuteCommand.Execute();
	}
}
