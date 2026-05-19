// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Tarcopy : ModuleRules
{
    public Tarcopy(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "ChaosVehicles",
            "EnhancedInput",
            "PhysicsCore",
            "AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "AudioCapture",
            "AudioCaptureCore",
            "AudioCaptureWasapi",
            "AudioMixer",
            "SignalProcessing",
            "Niagara",
        });

        PublicIncludePaths.AddRange(new[]
        {
            "Tarcopy"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "UMG",
            "Slate",
            "SlateCore",
            "NetCore",
            "EOSVoiceChatPTT"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
