// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#include "ExampleAssetManager.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsPlatformMisc.h"
#endif

bool UExampleAssetManager::TryParseReleaseVersion(const FString& StringValue, FExampleVersion& OutReleaseVersion)
{
	if (!StringValue.IsEmpty())
	{
		TArray<FString> Tokens;
		StringValue.ParseIntoArray(Tokens, TEXT("."));
		if (Tokens.Num() == 2)
		{
			// Parse the version number
			OutReleaseVersion.MajorVersion = FCString::Atoi(*Tokens[0]);
			OutReleaseVersion.MinorVersion = FCString::Atoi(*Tokens[1]);
			return true;
		}
	}

	return false;
}

bool UExampleAssetManager::TryGetReleaseVersionFromEnvVar(FExampleVersion& OutReleaseVersion)
{
	// Try to retrieve the release version from environment variable
	const FString EnvVarValue = FPlatformMisc::GetEnvironmentVariable(TEXT("EXAMPLE_RELEASE_VERSION"));
	if (EnvVarValue.Len() > 0)
	{
		if (TryParseReleaseVersion(EnvVarValue, OutReleaseVersion))
		{
			UE_LOG(LogTemp, Log, TEXT("Parsed release version %s from environment variable."), *OutReleaseVersion.ToString());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to parse release version from environment variable: %s."), *EnvVarValue);
		}
	}

	return false;

}

bool UExampleAssetManager::TryGetReleaseVersionFromCommandLine(FExampleVersion& OutReleaseVersion)
{
	// Try to retrieve the release version from command line.
	FString CommandLineValue;
	if (FParse::Value(FCommandLine::Get(), TEXT("ExampleReleaseVersion="), CommandLineValue))
	{
		if (TryParseReleaseVersion(CommandLineValue, OutReleaseVersion))
		{
			UE_LOG(LogTemp, Log, TEXT("Parsed release version %s from environment variable."), *OutReleaseVersion.ToString());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to parse release version from environment variable: %s."), *CommandLineValue);
		}
	}
	
	return false;
}

bool UExampleAssetManager::TryGetReleaseVersionFromConfig(FExampleVersion& OutReleaseVersion)
{
	// Retrieve release version from DefaultGame.ini and treat as release version.
	FString ExampleReleaseVersion;
	if (GConfig->GetString(TEXT("MyGame"), TEXT("ExampleReleaseVersion"), ExampleReleaseVersion, GGameIni))
	{
		if (TryParseReleaseVersion(ExampleReleaseVersion, OutReleaseVersion))
		{
			UE_LOG(LogTemp, Log, TEXT("Parsed release version %s from Config ExampleReleaseVersion."), *OutReleaseVersion.ToString());
			return true;
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Failed to parse release version from Config ExampleReleaseVersion: %s."), *ExampleReleaseVersion);
		}
	}

	return false;
}

/** 
 * Public static function so that anyone can retrieve the release version.
 * In general, assets and other systems shouldn't get this release version,
 * but rely on AssetManager to get their intended version range instead.
 *
 * You can consider moving the getter function to a small code module so that
 * other modules/plugins can import this getter function.
 */
FExampleVersion UExampleAssetManager::GetReleaseVersion()
{
	static bool bVersionCached = false;
	static FExampleVersion CachedVersion;

	// If version was cached before, return it now
	if (bVersionCached)
	{
		return CachedVersion;
	}

	FExampleVersion OutVersion;

	// Try getting the version info from various sources in order of priority. If you supply the version info from command-line, be aware
	// that UAT doesn't automatically forward original cmd args to the command of each build step, so it's limitedly useful unless you
	// manually trigger the build step commands or modify UAT. Env vars are quickest to get external values working, while command line
	// can be useful if you are able to customize build tools.
	if (TryGetReleaseVersionFromCommandLine(OutVersion) || TryGetReleaseVersionFromEnvVar(OutVersion) || TryGetReleaseVersionFromConfig(OutVersion))
	{
		CachedVersion = OutVersion;
		bVersionCached = true;
		return CachedVersion;
	}

	checkf(false, TEXT("Failed to parse release version from command line and ini. Expected in DefaultGame.ini: ProjectVersion=X.Y"));
	return FExampleVersion(0, 0);
}

bool UExampleAssetManager::DoesVersionRangeInclude(const FExampleVersionRange& VersionRange)
{
	// Get target release version
	const FExampleVersion ReleaseVersion = GetReleaseVersion();
	// Check if version range includes it
	return VersionRange.DoesRangeInclude(ReleaseVersion);
}

#if WITH_EDITOR
void UExampleAssetManager::ApplyPrimaryAssetLabels()
{
	Super::ApplyPrimaryAssetLabels();

	// Get target release version
	const FExampleVersion TargetReleaseVersion = GetReleaseVersion();
	UE_LOG(LogTemp, Log, TEXT("UExampleAssetManager::ApplyPrimaryAssetLabels START - Release version = %s"), *TargetReleaseVersion.ToString());

	// Retrieve list of all primary asset types.
	TArray<FPrimaryAssetTypeInfo> AssetTypeInfoList;
	GetPrimaryAssetTypeInfoList(AssetTypeInfoList);

	// Parse them and update their labels depending on whether we want to include it in this build.
	for (const FPrimaryAssetTypeInfo& AssetTypeInfo : AssetTypeInfoList)
	{
		// Gather and iterate all assets
		TArray<FAssetData> AllAssetsOfType;
		GetPrimaryAssetDataList(AssetTypeInfo.PrimaryAssetType, AllAssetsOfType);
		UE_LOG(LogTemp, Log, TEXT("  Found %d assets of primary asset type '%s'"), AllAssetsOfType.Num(), *AssetTypeInfo.PrimaryAssetType.ToString());

		// Parse all gathered assets of this asset type
		for (const FAssetData& AssetData : AllAssetsOfType)
		{
			const FPrimaryAssetId PrimaryAssetId = AssetData.GetPrimaryAssetId();

			// Of the registered primary asset types, we don't require the following types to have versioning info.
			static const TArray<FName> UnversionedPrimaryAssetTypes{"Map", "PrimaryAssetLabel", "GameFeatureData"};
			const bool bRequiresVersionRange = !UnversionedPrimaryAssetTypes.Contains(PrimaryAssetId.PrimaryAssetType);
			if (!bRequiresVersionRange)
			{
				// Leave their cook rule as is
				continue;
			}

			// Version info lookup:
			// - UPROPERTY(AssetRegistrySearchable) will have stored the property value as key-value pair, for example AExampleActor
			// - Alternatively, the GetAssetRegistryTags() override will have stored the key-value pair like in UExampleDataAsset
			FExampleVersionRange AssetVersionRange;
			if (AssetData.GetTagValue<FExampleVersionRange>(FExampleVersionRange::AssetTagName, AssetVersionRange))
			{
				// Decide whether to include asset based on current release version and the asset's version range
				const bool bShouldIncludeAsset = AssetVersionRange.DoesRangeInclude(TargetReleaseVersion);

				// Warning log level is just to highlight these messages for the tutorial when executing from shell. In production you wouldn't do this.
				UE_LOG(LogTemp, Warning, TEXT("  Asset '%s' has release versioning info: %s"), *AssetData.GetObjectPathString(), *AssetVersionRange.ToString());
				UE_LOG(LogTemp, Warning, TEXT("  --> %s asset '%s'"), bShouldIncludeAsset ? TEXT("Including") : TEXT("Excluding"), *AssetData.GetObjectPathString());

				// Override the cook rule based on that decision
				FPrimaryAssetRules Rules = GetPrimaryAssetRules(PrimaryAssetId);
				Rules.CookRule = bShouldIncludeAsset ? EPrimaryAssetCookRule::AlwaysCook : EPrimaryAssetCookRule::NeverCook;
				SetPrimaryAssetRules(PrimaryAssetId, Rules);
			}
			else
			{
				// This error message will fail the cook
				UE_LOG(LogTemp, Error, TEXT("  Asset '%s' did NOT have a release version as asset tag!"), *AssetData.GetObjectPathString());
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("UExampleAssetManager::ApplyPrimaryAssetLabels END"));
}
#endif
