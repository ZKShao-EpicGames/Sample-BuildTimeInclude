// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;
using EpicGames.Core;
using UnrealBuildBase;
using Microsoft.Extensions.Logging;
using System;

public class BuildTimeIncludeTarget : TargetRules
{
    public BuildTimeIncludeTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("BuildTimeInclude");

        // Override the build environment: this lets us dynamically enable plugins and raise bWarningsAsErrors
        //bOverrideBuildEnvironment = true;
        BuildEnvironment = TargetBuildEnvironment.Unique;

        // Whether warnings should cause build failures. Set to true if misconfigured GameFeature plugins should prevent a build.
        // Setting it to true would be safer, so that newly added plugins must have .uplugin files with expected versioning values.
        // Set to false if you don't need the version checking to be strict.
        bWarningsAsErrors = true;

        BuildTimeIncludeTarget.ConfigureGameFeaturePlugins(Target, Logger, ProjectFile, DisablePlugins, EnablePlugins);
    }

    // Struct to parse release version into with CommandLine.ParseArguments
    public class ReleaseVersionArgument
    {
        [CommandLine("-ExampleReleaseVersion")]
        public string ExampleReleaseVersion = null;
    }

    // Release version class with utility functions
    public class ReleaseVersion
    {
        // Release version value
        public int MajorVersion;
        public int MinorVersion;

        public ReleaseVersion(int Major, int Minor)
        {
            MajorVersion = Major;
            MinorVersion = Minor;
        }

        // Compare whether one release version is higher than the other by comparing components
        public static int Compare(ReleaseVersion Reference, ReleaseVersion Value)
        {
            int Sign = Math.Sign(Value.MajorVersion - Reference.MajorVersion);
            return Sign != 0 ? Sign : Math.Sign(Value.MinorVersion - Reference.MinorVersion);
        }

        // Parse a string like X.Y into Major and Minor components
        public static ReleaseVersion Parse(string Value)
        {
            string[] Tokens = Value.Split('.', 2);
            if (Tokens.Length == 2)
            {
                ReleaseVersion Result = new ReleaseVersion(0, 0);
                Result.MajorVersion = int.Parse(Tokens[0]);
                Result.MinorVersion = int.Parse(Tokens[1]);
                return Result;
            }
            return null;
        }
    }

    // Try to get release version from the -ExampleReleaseVersion commandline argument.
    // Not very compatible with RunUAT because it doesn't pass arguments onto the cook step.
    public static bool GetReleaseVersionFromCommandLine(ILogger Logger, out ReleaseVersion Version)
    {
        ReleaseVersionArgument ReleaseVersionArg = new ReleaseVersionArgument();
        UnrealBuildTool.CommandLine.ParseArguments(Environment.GetCommandLineArgs(), ReleaseVersionArg);
        if (ReleaseVersionArg.ExampleReleaseVersion != null)
        {
            try
            {
                Version = ReleaseVersion.Parse(ReleaseVersionArg.ExampleReleaseVersion);
                return true;
            }
            catch
            {
                Logger.LogError("Failed to parse project version {Arg0} into Major and Minor number.", ReleaseVersionArg.ExampleReleaseVersion);
                throw new BuildException("Failed in GetReleaseVersionFromCommandLine()");
            }
        }

        Version = null;
        return false;
    }

    // Try to get release version from system environment variables.
    // Recommended method to override the DefaultGame.ini value for a single build.
    public static bool GetReleaseVersionFromEnvVar(ILogger Logger, out ReleaseVersion Version)
    {
        string EnvVar = System.Environment.GetEnvironmentVariable("EXAMPLE_RELEASE_VERSION");
        if (EnvVar != null)
        {
            try
            {
                Version = ReleaseVersion.Parse(EnvVar);
                return true;
            }
            catch
            {
                Logger.LogError("Failed to parse environment variable value {Arg0} into Major and Minor number.", EnvVar);
                throw new BuildException("Failed in GetReleaseVersionFromEnvVar()");
            }
        }

        Version = null;
        return false;
    }

    // Get the release version from DefaultGame.ini
    public static bool GetReleaseVersionFromConfig(ILogger Logger, FileReference ProjectFile, out ReleaseVersion Version)
    {
        FileReference GameIni = FileReference.Combine(ProjectFile.Directory, "Config", "DefaultGame.ini");
        if (GameIni == null || !FileReference.Exists(GameIni))
        {
            Logger.LogError("DefaultGame.ini was not found in Config.");
            throw new BuildException("Failed in GetReleaseVersionFromConfig()");
        }

        ConfigFile GameConfigFile = new ConfigFile(GameIni);
        ConfigFileSection MyGameSection;
        ConfigLine ExampleReleaseVersionLine;
        if (!GameConfigFile.TryGetSection("MyGame", out MyGameSection) ||
            !MyGameSection.TryGetLine("ExampleReleaseVersion", out ExampleReleaseVersionLine))
        {
            Logger.LogError("DefaultGame.ini did not contain a [MyGame] section or ExampleReleaseVersion line.");
            throw new BuildException("Failed in GetReleaseVersionFromConfig()");
        }

        try
        {
            Version = ReleaseVersion.Parse(ExampleReleaseVersionLine.Value);
            return true;
        }
        catch
        {
            Logger.LogError("Failed to parse project version {Arg0} into Major and Minor number.", ExampleReleaseVersionLine.Value);
            throw new BuildException("Failed in GetReleaseVersionFromConfig()");
        }
    }

    // Get the target release version, prioritizing environment var, then Config
    public static bool GetTargetReleaseVersion(ILogger Logger, FileReference ProjectFile, out ReleaseVersion Version)
    {
        return GetReleaseVersionFromEnvVar(Logger, out Version) || GetReleaseVersionFromConfig(Logger, ProjectFile, out Version);
    }

    public static void ConfigureGameFeaturePlugins(TargetInfo Target, ILogger Logger, FileReference ProjectFile, List<string> OutDisablePlugins, List<string> OutEnablePlugins)
    {
        // Parse release version for this build. Command line argument takes precedence, otherwise use DefaultGame.ini ProjectVersion
        ReleaseVersion TargetVersion;
        GetTargetReleaseVersion(Logger, ProjectFile, out TargetVersion);
        Logger.LogInformation("Evaluating GameFeaturePlugins based on release version v{Arg0}.{Arg1} and configuration {Arg2}",
            TargetVersion.MajorVersion, TargetVersion.MinorVersion, Target.Configuration.ToString());

        // We will explore the Plugins/GameFeatures folder
        DirectoryReference GameFeaturePluginsDir = DirectoryReference.Combine(Target.ProjectFile.Directory, "Plugins", "GameFeatures");
        if (DirectoryReference.Exists(GameFeaturePluginsDir))
        {
            // Iterate over all plugins in the folder
            foreach (FileReference PluginFile in PluginsBase.EnumeratePlugins(GameFeaturePluginsDir))
            {
                if (PluginFile == null || !FileReference.Exists(PluginFile))
                {
                    continue;
                }

                string PluginName = PluginFile.GetFileNameWithoutExtension();
                bool bEnabled = false;
                try
                {
                    // Parse uplugin file as JsonObject, which we'll use to parse our custom versioning values. 
                    // Also parse it as PluginDescriptor, which will internally parse common descriptor values.
                    JsonObject RawObject = JsonObject.Read(PluginFile);
                    PluginDescriptor Descriptor = new PluginDescriptor(RawObject, PluginFile);

                    // Validate that GameFeaturePlugins are disabled by default
                    if (!Descriptor.bEnabledByDefault.HasValue || Descriptor.bEnabledByDefault.Value == true)
                    {
                        Logger.LogWarning("GameFeaturePlugin {Arg0}, does not set EnabledByDefault to false. This is required for GameFeaturePlugins in this project.", PluginName);
                        throw new Exception("Missing required EnabledByDefault: false in .uplugin");
                    }

                    // Get intro version string from uplugin
                    string IntroVersionStr;
                    if (!RawObject.TryGetStringField("IntroVersion", out IntroVersionStr))
                    {
                        Logger.LogWarning("GameFeaturePlugin {Arg0}, does not specify an IntroVersion. This is required for GameFeaturePlugins in this project.", PluginName);
                        throw new Exception("Missing required IntroVersion field in .uplugin");
                    }

                    // Parse intro version from string. Fail build if it didn't parse successfully.
                    ReleaseVersion IntroVersion = ReleaseVersion.Parse(IntroVersionStr);
                    if (IntroVersion == null)
                    {
                        throw new BuildException("Failed to parse IntroVersion {Arg0} for {Arg1}", IntroVersionStr, PluginName);
                    }

                    // Get sunset version values from uplugin, the value is optional
                    bool bHasSunsetVersion = false;
                    string SunsetVersionStr = "";
                    if (!RawObject.TryGetBoolField("HasSunsetVersion", out bHasSunsetVersion) ||
                        bHasSunsetVersion && !RawObject.TryGetStringField("SunsetVersion", out SunsetVersionStr))
                    {
                        Logger.LogWarning("GameFeaturePlugin {Arg0}, does not specify HasSunsetVersion == fales, or missing the SunsetVersion. This is required for GameFeaturePlugins in this project.", PluginName);
                        throw new Exception("Missing required HasSunsetVersion field in .uplugin, or HasSunsetVersion == true and missing SunsetVersion.");
                    }

                    // Parse sunset version (optional)
                    ReleaseVersion SunsetVersion = bHasSunsetVersion ? ReleaseVersion.Parse(SunsetVersionStr) : null;
                    if (bHasSunsetVersion && SunsetVersion == null)
                    {
                        throw new BuildException("Failed to parse SunsetVersion {Arg0} for {Arg1}", SunsetVersionStr, PluginName);
                    }

                    // Do the version check
                    if (ReleaseVersion.Compare(IntroVersion, TargetVersion) >= 0 && (!bHasSunsetVersion || ReleaseVersion.Compare(SunsetVersion, TargetVersion) < 0))
                    {
                        Logger.LogWarning("GameFeaturePlugin {Arg0} passes release version checks, enabling!", PluginName);
                        bEnabled = true;
                    }
                    else
                    {
                        Logger.LogWarning("GameFeaturePlugin {Arg0} did NOT pass release version checks.", PluginName);
                    }
                }
                catch (Exception ParseException)
                {
                    Logger.LogWarning("Failed to parse GameFeaturePlugin file {Arg0}, disabling. Exception: {Arg1}", PluginName, ParseException.Message);
                    bEnabled = false;
                }

                Logger.LogWarning("GameFeaturePlugin {Arg0} dynamically enabled? Outcome = {Arg1}", PluginName, bEnabled);
                if (bEnabled)
                {
                    // If we decided to enable the plugin, add it to the dynamic list to enable
                    OutEnablePlugins.Add(PluginName);
                }
            }
        }

        foreach (string PluginName in OutEnablePlugins)
        {
            Logger.LogWarning("Enabled plugin: {Arg1}", PluginName);
        }
    }
}
