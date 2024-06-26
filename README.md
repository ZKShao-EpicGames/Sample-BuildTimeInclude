Example project part of the tutorial "Build-time Asset and Plugin Exclusion" for Unreal Engine
https://dev.epicgames.com/community/learning/tutorials/Kp1k/unreal-engine-build-time-asset-and-plugin-exclusion

Project is built targeting Unreal Engine 5.3, built from source
https://dev.epicgames.com/documentation/en-us/unreal-engine/building-unreal-engine-from-source
https://github.com/EpicGames/UnrealEngine/tree/5.3

Instructions:
- Download and compile Unreal Engine from source, must be UE 5.3 or higher
- Associate the project with that Unreal Engine version by right clicking BuildTimeInclude.uproject > Switch Unreal Engine version
- Modify RunBuildCookStage.bat to point to that Unreal Engine version's location for RunUAT.bat
- Play with running RunBuildCookStage.bat with different values for EXAMPLE_RELEASE_VERSION
- On success, run RunStagedClient.bat to launch the built client or browse to it under Saved/StagedBuilds.

More information is provided in the linked tutorial.
