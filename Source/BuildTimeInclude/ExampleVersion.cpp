// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#include "ExampleVersion.h"

int8 FExampleVersion::Compare(const FExampleVersion& Reference, const FExampleVersion& Value)
{
    // Compare by major number first, and only then by minor number if needed
    const int8 Sign = FMath::Sign<int8>(Value.MajorVersion - Reference.MajorVersion);
    return Sign != 0 ? Sign : FMath::Sign<int8>(Value.MinorVersion - Reference.MinorVersion);
}
