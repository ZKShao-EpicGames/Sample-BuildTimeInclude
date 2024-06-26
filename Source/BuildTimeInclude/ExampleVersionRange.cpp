// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#include "ExampleVersionRange.h"
#include "JsonUtilities.h"

FName FExampleVersionRange::AssetTagName = FName("VersionRange");

FString FExampleVersionRange::ToAssetTagValue(const FExampleVersionRange& Range)
{
    // Default struct to string, similar to FStructProperty::ExportText_Internal.
    FString OutVal;
    FExampleVersionRange::StaticStruct()->ExportText(OutVal, &Range, nullptr, nullptr, 0, nullptr, false);
    return OutVal;
}

FExampleVersionRange FExampleVersionRange::FromAssetTagValue(const FString& Value)
{
    // Default UE struct from string
    FExampleVersionRange OutVal;
    FExampleVersionRange::StaticStruct()->ImportText(*Value, &OutVal, nullptr, 0, nullptr, "");
    return OutVal;
}

FString FExampleVersionRange::ToString() const
{
    return FString::Printf(TEXT("MinVersion=v%d.%d | MaxVersion(enabled=%d)=v%d.%d"), IntroVersion.MajorVersion, IntroVersion.MinorVersion, bHasSunsetVersion, SunsetVersion.MajorVersion, SunsetVersion.MinorVersion);
}

bool FExampleVersionRange::DoesRangeInclude(const FExampleVersion& Version) const
{
    return (FExampleVersion::Compare(IntroVersion, Version) >= 0) && (!bHasSunsetVersion || FExampleVersion::Compare(SunsetVersion, Version) < 0);
}

void LexFromString(FExampleVersionRange& OutValue, const TCHAR* Buffer)
{
    FExampleVersionRange::StaticStruct()->ImportText(Buffer, &OutValue, nullptr, 0, nullptr, "");
}
