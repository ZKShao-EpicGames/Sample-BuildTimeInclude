// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ExampleVersionRange.h"
#include "ExampleAssetManager.generated.h"

/**
 * Example implementation of a custom asset manager, which is consulted at cook-time to help decide which 
 * assets to include in the build. The project's asset manager should be set to this class either via
 * Project Settings or in DefaultEngine.ini directly.
 */
UCLASS()
class BUILDTIMEINCLUDE_API UExampleAssetManager : public UAssetManager
{
	GENERATED_BODY()

private:
	static bool TryParseReleaseVersion(const FString& StringValue, FExampleVersion& OutReleaseVersion);
	static bool TryGetReleaseVersionFromEnvVar(FExampleVersion& OutReleaseVersion);
	static bool TryGetReleaseVersionFromCommandLine(FExampleVersion& OutReleaseVersion);
	static bool TryGetReleaseVersionFromConfig(FExampleVersion& OutReleaseVersion);

public:
	static FExampleVersion GetReleaseVersion();
	static bool DoesVersionRangeInclude(const FExampleVersionRange& VersionRange);

#if WITH_EDITOR
	virtual void ApplyPrimaryAssetLabels() override;
#endif
	
};
