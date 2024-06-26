// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ExampleVersionRange.h"
#include "ExampleDataAsset.generated.h"

/**
 * Example of a data asset class which will be considered for inclusion at cook-time.
 */
UCLASS()
class BUILDTIMEINCLUDE_API UExampleDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

	// Return an id with custom PrimaryAssetType that asset manager will look for to consider assets with this base class for inclusion
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// Add version range as asset registry tags: class agnostic values accessible to asset manager to decide the asset's inclusion
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;

	// The range of game versions in which this data asset should be included. Is checked in ExampleAssetManager.
	// Since ExampleAssetManager expects the version info with key 'VersionRange' instead of 'MyVersionRange', this
	// class demonstrates manually writing key-value pairs. ExampleActor demonstrates the other approach: AssetRegistrySearchable.
	UPROPERTY(EditDefaultsOnly)
	FExampleVersionRange MyVersionRange;
	
};
