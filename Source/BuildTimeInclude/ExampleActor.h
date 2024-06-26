// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ITargetPlatform.h"
#include "ExampleVersionRange.h"
#include "UObject/ObjectSaveContext.h"
#include "ExampleActor.generated.h"

UCLASS()
class BUILDTIMEINCLUDE_API AExampleActor : public AActor
{
	GENERATED_BODY()
	
public:
	AExampleActor();

	// Return an id with custom PrimaryAssetType that asset manager will look for
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	// If a level contains this actor, you will see a message logged at BeginPlay. In the standalone build, check for the message.
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Evaluated whenever an actor CDO or instance is loaded
	virtual void PostLoad() override;
	// Evaluated whenever saved, including at cook-time for cooked asset
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif

	// The range of game versions in which this actor should be included. Is checked in ExampleAssetManager for whether to include the blueprint class
	// during packaging. Is checked for this actor instance for whether map placed instances should be included in the cooked map. Be aware that you can
	// run into errors if the actor is excluded when it's referenced by other included content.
	UPROPERTY(EditDefaultsOnly, AssetRegistrySearchable)
	FExampleVersionRange VersionRange;

};
