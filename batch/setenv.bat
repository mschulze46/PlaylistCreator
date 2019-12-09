@rem Project root path
@set WORKSPACE_ID=PlaylistCreator3
@set PROJECT_ID=PC3

@set DEV_DIR=p:\
@set CODE_DIR=p:\%WORKSPACE_ID%\

@rem Visual Studio
@set DEVENV_PATH="%ProgramFiles%\Microsoft Visual Studio 9.0\Common7\IDE\devenv.exe"

@rem HTML Help Workshop
@set HHC_PATH="%ProgramFiles%\HTML Help Workshop\hhc.exe"

@rem Nullsoft Installer
@set NSIS_PATH="%ProgramFiles%\NSIS\makensis.exe"

@rem SubWCRev
@set SUBWCREV_PATH="%ProgramFiles%\TortoiseSVN\bin\SubWCRev.exe"

@rem Derived paths
@set OUTPUT_DIR=o:\%WORKSPACE_ID%\
@set OUTPUT_RELEASE_DIR=%OUTPUT_DIR%Release\
@set OUTPUT_DEBUG_DIR=%OUTPUT_DIR%Debug\
@set HELP_DIR=%CODE_DIR%help\
@set INSTALLER_DIR=%CODE_DIR%install\
