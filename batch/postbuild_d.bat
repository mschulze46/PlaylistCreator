@echo off
call p:\PlaylistCreator3\batch\setenv.bat


@rem Copy binaries to output dir
xcopy "%CODE_DIR%dll\*.dll" "%OUTPUT_DEBUG_DIR%" /Y
xcopy "%CODE_DIR%help\*.chm" "%OUTPUT_DEBUG_DIR%" /Y

echo ******************************************** POST-BUILD COMPLETE (DEBUG)
