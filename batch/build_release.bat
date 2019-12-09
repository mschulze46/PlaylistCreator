@echo off
echo Build Release

call setenv.bat


%DEVENV_PATH% "%CODE_DIR%\%WORKSPACE_ID%.sln" /rebuild Release

echo *************************************************** BUILD COMPLETE

call postbuild_r.bat
