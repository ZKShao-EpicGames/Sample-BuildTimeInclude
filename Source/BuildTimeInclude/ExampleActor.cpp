// Example project which build-time cooks actor classes, map actors, data assets and entire plugins based on game version number and build type.

#include "ExampleActor.h"
#include "ExampleAssetManager.h"
#include "Components/GameFrameworkComponentManager.h"

AExampleActor::AExampleActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

FPrimaryAssetId AExampleActor::GetPrimaryAssetId() const
{
	FPrimaryAssetId MyId = Super::GetPrimaryAssetId();
	MyId.PrimaryAssetType = FPrimaryAssetType(TEXT("ExampleActor"));
	MyId.PrimaryAssetName = GetClass()->GetFName();
	return MyId;
}

void AExampleActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Actor '%s' of type '%s' reporting for duty!"), *GetPathName(), *GetClass()->GetName());

	// Register ourselves with the modular gameplay manager. This will now add components to this actor when
	// a GameFeaturePlugin specifies it. For example: this ensures hats are loaded if (and only if) the 
	// ExampleGameFeaturePlugin was cooked and packaged as part of a build's release version.
	if (UGameFrameworkComponentManager* ComponentManager = GetGameInstance()->GetSubsystem<UGameFrameworkComponentManager>())
	{
		ComponentManager->AddReceiver(this);
	}
}

#if WITH_EDITOR
void AExampleActor::PostLoad()
{
	Super::PostLoad();

	// If cooking and target release version excludes this class, mark self as transient so the object won't be saved.
	// This will apply to blueprint class default objects and map instances. Transient objects are not serialized when
	// their outer package is cooked, see FSaveContext::GetSaveableStatusNoOuter. For example, actor instances that are 
	// transient are not saved when the outer map is cooked.
	if (IsRunningCookCommandlet() && !UExampleAssetManager::DoesVersionRangeInclude(VersionRange))
	{
		SetFlags(EObjectFlags::RF_Transient);
		UE_LOG(LogTemp, Warning, TEXT("Actor '%s' of type '%s' marking self as transient to avoid save"), *GetPathName(), *GetClass()->GetName());
	}
}

void AExampleActor::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	if (ObjectSaveContext.IsCooking())
	{
		// Check actor's VersionRange against the version being cooked for	
		if (!UExampleAssetManager::DoesVersionRangeInclude(VersionRange))
		{
			// If the class is version excluded yet still the actor instance is about to be saved as part of a package 
			// (like a map containing the actor instance), print an error here. The presence of an error log will fail 
			// the cook. This is a final attempt to catch a version excluded blueprint class's instance to prevent a 
			// possible data leak. To see its effectiveness, try commenting out the PostLoad() behavior to mark actor
			// instance transient and then try to cook a map with a release version where the actor class should be excluded.
			UE_LOG(LogTemp, Error, TEXT("Actor '%s' was cooked to '%s' despite not being version compatible! Make sure it's not referenced by another primary asset or force cooked in some way!"), *GetPathName(), ObjectSaveContext.GetTargetFilename());
		}
	}
}
#endif
