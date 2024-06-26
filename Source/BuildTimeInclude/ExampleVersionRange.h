// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ExampleVersion.h"
#include "ExampleVersionRange.generated.h"

/**
 * Used to represent a range of game versions AND build types in which an asset is to be included
 * when cooking and packaging. You can come up with your own parameters and implementing their
 * checking logic in ExampleAssetManager. This struct is used for individual assets. For plugins,
 * see BuildTimeInclude.Target.cs. 
 * 
 * In this example project the same values are checked for plugins, but the values are specified in
 * the .uplugin files.
 */
USTRUCT(BlueprintType)
struct BUILDTIMEINCLUDE_API FExampleVersionRange
{
	GENERATED_USTRUCT_BODY()

	// The key that assets store their version range as and the asset manager will look for.
	static FName AssetTagName;
	// Custom to string implementation. Matches the FStructProperty::ExportText_Internal() implementation.
	static FString ToAssetTagValue(const FExampleVersionRange& Range);
	// Custom from string implementation. Can parse what ToAssetTagValue() outputs.
	static FExampleVersionRange FromAssetTagValue(const FString& Value);

	// Human readable string representation
	FString ToString() const;

	// Whether a specific version is contained within the configured (intro version, optional sunset version) range.
	bool DoesRangeInclude(const FExampleVersion& Version) const;

	// If the build's game version is at least this, consider for inclusion.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FExampleVersion IntroVersion = FExampleVersion(0, 0);

	// Whether the content has a 'sunset version': a last version to be included in.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasSunsetVersion = false;

	// If the build's release version is at or past this, don't include it. The SunsetVersion itself is exclusive.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasSunsetVersion"))
	FExampleVersion SunsetVersion = FExampleVersion(99999, 0);
	
};

BUILDTIMEINCLUDE_API void LexFromString(FExampleVersionRange& OutValue, const TCHAR* Buffer);