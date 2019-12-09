@echo off
echo Build Installer

call setenv.bat


:_copy_binaries

xcopy "%OUTPUT_RELEASE_DIR%*.exe" "%INSTALLER_DIR%\bin\" /Y
xcopy "%OUTPUT_RELEASE_DIR%*.dll" "%INSTALLER_DIR%\bin\" /Y
xcopy "%OUTPUT_RELEASE_DIR%*.chm" "%INSTALLER_DIR%\bin\" /Y

echo *************************************************** BINARIES COPIED


:_build_installer

:: Fetch SVN revision number via SubWCRev
%SUBWCREV_PATH% "%CODE_DIR%." "%INSTALLER_DIR%\svnrev.tpl" "%INSTALLER_DIR%\svnrev.txt"

:: Compile installer script
%NSIS_PATH% /V4 "%INSTALLER_DIR%\%PROJECT_ID%.nsi"

echo *************************************************** BUILD COMPLETE


:_exit

echo _
