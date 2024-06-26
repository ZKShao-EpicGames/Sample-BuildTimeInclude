// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using static BuildTimeIncludeTarget;
using System.Collections.Generic;

public class BuildTimeIncludeEditorTarget : TargetRules
{
	public BuildTimeIncludeEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BuildTimeInclude");

        // Use unique build environment: this lets us include plugins at build-time by adding their name to the "EnablePlugins" property.
        // Requires building the engine from source.
        BuildEnvironment = TargetBuildEnvironment.Unique;

        // Whether warnings should cause build failures. Set to true if misconfigured GameFeature plugins should prevent a build.
        // Setting it to true would be safer, so that newly added plugins must have .uplugin files with expected versioning values.
        // Set to false if you don't need the version checking to be strict.
        bWarningsAsErrors = true;

        BuildTimeIncludeTarget.ConfigureGameFeaturePlugins(Target, Logger, ProjectFile, DisablePlugins, EnablePlugins);
    }
}
