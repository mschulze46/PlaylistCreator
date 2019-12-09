@echo off
call p:\PlaylistCreator3\batch\setenv.bat


@rem Copy binaries to output dir
xcopy "%CODE_DIR%dll\*.dll" "%OUTPUT_RELEASE_DIR%" /Y
xcopy "%CODE_DIR%help\*.chm" "%OUTPUT_RELEASE_DIR%" /Y

echo ******************************************** POST-BUILD COMPLETE (RELEASE)
