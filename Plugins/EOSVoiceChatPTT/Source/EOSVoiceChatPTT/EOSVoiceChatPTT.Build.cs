using UnrealBuildTool;

public class EOSVoiceChatPTT : ModuleRules
{
	public EOSVoiceChatPTT(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"VoiceChat",
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"OnlineSubsystemEOS",
			"EOSVoiceChat",
			"EOSShared",
			"EOSSDK",
			"Slate",
			"SlateCore",
			"AudioCaptureCore"
		});

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDependencyModuleNames.Add("AudioCaptureWasapi");
		}
	}
}
