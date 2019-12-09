@echo off
echo Performing full deploy

call build_help.bat
call build_release.bat
call build_installer.bat
