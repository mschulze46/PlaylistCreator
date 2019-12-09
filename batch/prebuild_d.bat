@echo off
call p:\PlaylistCreator3\batch\setenv.bat


@rem SubWCRev: Get revision number of working copy
%SUBWCREV_PATH% "%CODE_DIR%." "%CODE_DIR%code\version.in" "%CODE_DIR%code\version.h"

echo ******************************************** PRE-BUILD COMPLETE (DEBUG)
