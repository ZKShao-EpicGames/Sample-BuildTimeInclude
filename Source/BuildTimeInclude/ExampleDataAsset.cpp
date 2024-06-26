// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#include "ExampleDataAsset.h"

FPrimaryAssetId UExampleDataAsset::GetPrimaryAssetId() const
{
	FPrimaryAssetId MyId = Super::GetPrimaryAssetId();
	MyId.PrimaryAssetType = FPrimaryAssetType(TEXT("ExampleDataAsset"));
	MyId.PrimaryAssetName = GetFName();
	return MyId;
}

void UExampleDataAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	Super::GetAssetRegistryTags(OutTags);

	const FName VersionRangeTagName = FExampleVersionRange::AssetTagName;
	const FString VersionRangeTagValue = FExampleVersionRange::ToAssetTagValue(MyVersionRange);
	FAssetRegistryTag ReleaseVersionTag(VersionRangeTagName, VersionRangeTagValue, UObject::FAssetRegistryTag::ETagType::TT_Hidden);
	OutTags.Add(ReleaseVersionTag);
}
