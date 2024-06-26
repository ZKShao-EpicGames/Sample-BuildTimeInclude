REM Uncomment the following line to set release version via environment var
set EXAMPLE_RELEASE_VERSION=4.0

REM Put your UE path here. It must be a source build in order to dynamically enable plugins.
"PATH_TO_UE5.3\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun -project="%cd%/BuildTimeInclude.uproject" -clientconfig=Development -build -cook -stage -pak
