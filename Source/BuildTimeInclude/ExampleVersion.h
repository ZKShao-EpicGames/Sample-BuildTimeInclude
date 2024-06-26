// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#pragma once

#include "CoreMinimal.h"
#include "ExampleVersion.generated.h"

// This is an example of a game version identifier using Major.Minor numbering: v2.3 etc
USTRUCT(BlueprintType)
struct BUILDTIMEINCLUDE_API FExampleVersion
{
	GENERATED_USTRUCT_BODY()

	FExampleVersion() : MajorVersion(1), MinorVersion(0) {}
	FExampleVersion(const int32 MajorVersion, const int32 MinorVersion) : MajorVersion(MajorVersion), MinorVersion(MinorVersion) {}

	FString ToString() const { return FString::Printf(TEXT("v%d.%d"), MajorVersion, MinorVersion); }

	// -1 if Value < Reference, 0 if Value == Reference, 1 if Value > Reference
	static int8 Compare(const FExampleVersion& Reference, const FExampleVersion& Value);

	// Major component of the version number {Major.Minor}
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MajorVersion = 0;

	// Minor component of the version number {Major.Minor}
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MinorVersion = 0;
};
