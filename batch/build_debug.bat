@echo off
echo Build Debug

call setenv.bat


%DEVENV_PATH% "%CODE_DIR%\%WORKSPACE_ID%.sln" /rebuild Debug

echo *************************************************** BUILD COMPLETE

call postbuild_d.bat
