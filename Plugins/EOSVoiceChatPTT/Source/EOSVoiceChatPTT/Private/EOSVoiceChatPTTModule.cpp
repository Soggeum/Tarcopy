#include "Modules/ModuleManager.h"

class FEOSVoiceChatPTTModule : public IModuleInterface
{
public:
	virtual void StartupModule() override {}
	virtual void ShutdownModule() override {}
};

IMPLEMENT_MODULE(FEOSVoiceChatPTTModule, EOSVoiceChatPTT)
