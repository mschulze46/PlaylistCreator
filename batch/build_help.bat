@echo off
echo Build Help %1

call setenv.bat

if "%1"=="" goto _build_all


:_build_lang

%HHC_PATH% %HELP_DIR%\%PROJECT_ID%_%1.hhp

echo *************************************************** LANGUAGE COMPLETE

goto _exit


:_build_all

call %CODE_DIR%batch\build_help.bat DE
call %CODE_DIR%batch\build_help.bat EN

echo *************************************************** BUILD COMPLETE


:_exit

echo _
