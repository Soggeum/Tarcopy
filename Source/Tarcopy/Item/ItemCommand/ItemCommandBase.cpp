#include "Item/ItemCommand/ItemCommandBase.h"

void UItemCommandBase::Execute(const FItemCommandContext& Context)
{
	if (bExecutable == false)
		return;

	OnExecute(Context);
}
