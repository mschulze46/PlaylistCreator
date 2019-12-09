; Playlist Creator installation script
; Written by Michael Schulze <mailto:michael.schulze@oddgravity.de>
;
; Requires NSIS >= 2.35

;-------------------------------------------
; includes
;-------------------------------------------

!include "MUI.nsh"
!include "svnrev.txt"


;-------------------------------------------
; variables
;-------------------------------------------

!define OG_COMPANY            "oddgravity"                        ; company name

!define OG_VERSION_MAJOR      "3"                                 ; e.g. "2"
!define OG_VERSION_MINOR      "3.6"                               ; e.g. "2.3"
!define OG_VERSION_FIX        "3.6.2"                             ; e.g. "2.3.4"
!define OG_VERSION_BUILD      "3.6.2.0"                           ; e.g. "2.3.4.5"

!define OG_PRODUCT            "Playlist Creator"                  ; product name (e.g. "MyProduct")
!define OG_PRODUCT_FULL       "${OG_PRODUCT} ${OG_VERSION_FIX}"   ; full product name (e.g. "MyProduct 1.2.3")

!define OG_REGKEY             "oddgravity\PlaylistCreator3"       ; registry key
!define OG_APP_FILENAME       "pc3.exe"                           ; application file name
!define OG_COPYRIGHT          "Copyright © oddgravity"            ; copyright
!define OG_OUTPUT_FILENAME    "PlaylistCreator3_Setup.exe"        ; output file name
!define OG_WEBSITE_URL        "http://www.oddgravity.de"          ; website URL

Var MUI_TEMP
Var STARTMENU_FOLDER


;-------------------------------------------
; resources
;-------------------------------------------

; install
!define MUI_ICON                          "res\install.ico"
!define MUI_HEADERIMAGE_BITMAP            "res\install-header.bmp"
!define MUI_WELCOMEFINISHPAGE_BITMAP      "res\install-wizard.bmp"

; uninstall
!define MUI_UNICON                        "res\uninstall.ico"
!define MUI_HEADERIMAGE_UNBITMAP          "res\uninstall-header.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP    "res\uninstall-wizard.bmp"


;-------------------------------------------
; version information metadata
;-------------------------------------------

VIProductVersion                  "${OG_VERSION_BUILD}"
VIAddVersionKey ProductName       "${OG_PRODUCT}"
VIAddVersionKey ProductVersion    "${OG_VERSION_BUILD}"
VIAddVersionKey Comments          "${OG_PRODUCT} Installer"
VIAddVersionKey CompanyName       "${OG_COMPANY}"
VIAddVersionKey LegalCopyright    "${OG_COPYRIGHT}"
VIAddVersionKey FileDescription   "${OG_PRODUCT} Win32 Installer"
VIAddVersionKey FileVersion       "${OG_VERSION_MINOR}"
VIAddVersionKey InternalName      "${OG_PRODUCT} Installer - Win32"
VIAddVersionKey LegalTrademarks   ""
VIAddVersionKey OriginalFilename  "${OG_APP_FILENAME}"
VIAddVersionKey PrivateBuild      ""
VIAddVersionKey SpecialBuild      ""


;-------------------------------------------
; configuration
;-------------------------------------------

; name and output file
Name "${OG_PRODUCT_FULL}"
OutFile "${OG_OUTPUT_FILENAME}"

; default install folder
InstallDir "$PROGRAMFILES\${OG_PRODUCT_FULL}"

; get install folder from registry if available
InstallDirRegKey HKCU "Software\{$OG_REGKEY}" ""

; set branding text
BrandingText "${OG_PRODUCT} ${OG_VERSION_FIX} Revision ${OG_SVN_REVISION}"

; [>= Vista] set startup execution level
RequestExecutionLevel user    ; UAC plugin will request admin level later on

; define whether to allow install to root dir
AllowRootDirInstall false

; set compressor
SetCompressor /SOLID /FINAL LZMA  ; use LZMA compressor, no further changes of compressor allowed

; define whether to show details during install/uninstall
ShowInstDetails show
ShowUninstDetails show

; miscellaneous
CRCCheck on
XPStyle on

; remember the installer language (should be set before the page macros)
!define MUI_LANGDLL_REGISTRY_ROOT       "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY        "Software\${OG_REGKEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME  "Installer Language"


;-------------------------------------------
; pages
;-------------------------------------------

  ;-------------------------------------------
  ; general
  ;-------------------------------------------

  !define MUI_HEADERIMAGE
  !define MUI_ABORTWARNING

  ;-------------------------------------------
  ; install
  ;-------------------------------------------

  !insertmacro MUI_PAGE_WELCOME

    !define MUI_LICENSEPAGE_CHECKBOX
  !insertmacro MUI_PAGE_LICENSE $(license)

  !insertmacro MUI_PAGE_DIRECTORY

    !define MUI_STARTMENUPAGE_DEFAULTFOLDER "${OG_PRODUCT_FULL}"
    !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
    !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${OG_REGKEY}"
    !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

  !insertmacro MUI_PAGE_INSTFILES

    !define MUI_FINISHPAGE_RUN
    !define MUI_FINISHPAGE_RUN_TEXT               $(IDS_FINISHPAGE_RUN)
    !define MUI_FINISHPAGE_RUN_FUNCTION           CustomFinishRun
    !define MUI_FINISHPAGE_RUN_CHECKED
    !define MUI_FINISHPAGE_SHOWREADME
    !define MUI_FINISHPAGE_SHOWREADME_TEXT        $(IDS_FINISHPAGE_SHOWREADME)
    !define MUI_FINISHPAGE_SHOWREADME_FUNCTION    CreateDesktopIcon
    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
    !define MUI_FINISHPAGE_LINK                   $(IDS_FINISHPAGE_LINK)
    !define MUI_FINISHPAGE_LINK_LOCATION          ${OG_WEBSITE_URL}
  !insertmacro MUI_PAGE_FINISH

  ;-------------------------------------------
  ; uninstall
  ;-------------------------------------------

  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH


;-------------------------------------------
; languages
;  note: language macros must be inserted *after* page macros
;-------------------------------------------

  ;-------------------------------------------
  ; supported languages
  ;-------------------------------------------

  !insertmacro MUI_LANGUAGE "English"     ; ID = 1033 (first language is the default language)
  !insertmacro MUI_LANGUAGE "German"      ; ID = 1031

  ;-------------------------------------------
  ; english
  ;-------------------------------------------

  ; license
  LicenseLangString license                 ${LANG_ENGLISH} "res\license_en.rtf"

  ; strings
  LangString IDS_FINISHPAGE_RUN             ${LANG_ENGLISH} "Launch ${OG_PRODUCT}"
  LangString IDS_FINISHPAGE_SHOWREADME      ${LANG_ENGLISH} "Create desktop shortcut"
  LangString IDS_FINISHPAGE_LINK            ${LANG_ENGLISH} "Visit the oddgravity website..."
  LangString IDS_HELP_FILE                  ${LANG_ENGLISH} "PC3_EN.chm"
  LangString IDS_STARTMENU_APP_LNK          ${LANG_ENGLISH} "${OG_PRODUCT_FULL}"
  LangString IDS_STARTMENU_APP_TIP          ${LANG_ENGLISH} "Launch ${OG_PRODUCT_FULL}"
  LangString IDS_STARTMENU_HELP_LNK         ${LANG_ENGLISH} "${OG_PRODUCT_FULL} Help"
  LangString IDS_STARTMENU_HELP_TIP         ${LANG_ENGLISH} "Launch ${OG_PRODUCT_FULL} Help"
  LangString IDS_STARTMENU_HOMEPAGE_LNK     ${LANG_ENGLISH} "www.oddgravity.de"
  LangString IDS_STARTMENU_HOMEPAGE_TIP     ${LANG_ENGLISH} "Visit website"
  LangString IDS_STARTMENU_RELNOTES_LNK     ${LANG_ENGLISH} "Release Notes"
  LangString IDS_STARTMENU_RELNOTES_TIP     ${LANG_ENGLISH} "Show Release Notes"
  LangString IDS_DESKTOP_SHORTCUT_LNK       ${LANG_ENGLISH} "${OG_PRODUCT_FULL}"
  LangString IDS_DESKTOP_SHORTCUT_TIP       ${LANG_ENGLISH} "Launch ${OG_PRODUCT_FULL}"

  ;-------------------------------------------
  ; german
  ;-------------------------------------------

  ; license
  LicenseLangString license                 ${LANG_GERMAN} "res\license_de.rtf"

  ; strings
  LangString IDS_FINISHPAGE_RUN             ${LANG_GERMAN} "${OG_PRODUCT} starten"
  LangString IDS_FINISHPAGE_SHOWREADME      ${LANG_GERMAN} "Verknüpfung auf dem Desktop erstellen"
  LangString IDS_FINISHPAGE_LINK            ${LANG_GERMAN} "Besuchen Sie die oddgravity Website..."
  LangString IDS_HELP_FILE                  ${LANG_GERMAN} "PC3_DE.chm"
  LangString IDS_STARTMENU_APP_LNK          ${LANG_GERMAN} "${OG_PRODUCT_FULL}"
  LangString IDS_STARTMENU_APP_TIP          ${LANG_GERMAN} "${OG_PRODUCT_FULL} starten"
  LangString IDS_STARTMENU_HELP_LNK         ${LANG_GERMAN} "${OG_PRODUCT_FULL} Hilfe"
  LangString IDS_STARTMENU_HELP_TIP         ${LANG_GERMAN} "${OG_PRODUCT_FULL} Hilfe starten"
  LangString IDS_STARTMENU_HOMEPAGE_LNK     ${LANG_GERMAN} "www.oddgravity.de"
  LangString IDS_STARTMENU_HOMEPAGE_TIP     ${LANG_GERMAN} "Website besuchen"
  LangString IDS_STARTMENU_RELNOTES_LNK     ${LANG_GERMAN} "Release Notes"
  LangString IDS_STARTMENU_RELNOTES_TIP     ${LANG_GERMAN} "Release Notes anzeigen"
  LangString IDS_DESKTOP_SHORTCUT_LNK       ${LANG_GERMAN} "${OG_PRODUCT_FULL}"
  LangString IDS_DESKTOP_SHORTCUT_TIP       ${LANG_GERMAN} "${OG_PRODUCT_FULL} starten"


;-------------------------------------------
; reserve files
;-------------------------------------------

; the following files should be inserted before other files in the data block
; note: keep these lines before any file command
!insertmacro MUI_RESERVEFILE_LANGDLL  ; language selection dialog
ReserveFile ${MUI_ICON}
ReserveFile ${MUI_HEADERIMAGE_BITMAP}
ReserveFile ${MUI_WELCOMEFINISHPAGE_BITMAP}
ReserveFile ${MUI_UNICON}
ReserveFile ${MUI_HEADERIMAGE_UNBITMAP}
ReserveFile ${MUI_UNWELCOMEFINISHPAGE_BITMAP}


;-------------------------------------------
; sections
;-------------------------------------------

Section "${OG_PRODUCT}" SecMain

  SectionIn 1 RO  ; read-only

  ; set installation path
  SetOutPath "$INSTDIR"

  ; define files to install
  File "bin\pc3.exe"
  File "bin\cximage.dll"
  File "bin\MediaInfo.dll"
  File "bin\PC3_DE.chm"
  File "bin\PC3_EN.chm"
  File "bin\ReleaseNotes.htm"
  File "bin\OddgravityWebsite.url"

  ; remember install folder
  WriteRegStr HKCU "Software\${OG_REGKEY}" "" $INSTDIR

  ; create start menu shortcuts
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(IDS_STARTMENU_APP_LNK).lnk"       "$INSTDIR\${OG_APP_FILENAME}"     "" "" "" "" "" "$(IDS_STARTMENU_APP_TIP)"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(IDS_STARTMENU_HELP_LNK).lnk"      "$INSTDIR\$(IDS_HELP_FILE)"       "" "" "" "" "" "$(IDS_STARTMENU_HELP_TIP)"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(IDS_STARTMENU_RELNOTES_LNK).lnk"  "$INSTDIR\ReleaseNotes.htm"       "" "" "" "" "" "$(IDS_STARTMENU_RELNOTES_TIP)"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\$(IDS_STARTMENU_HOMEPAGE_LNK).lnk"  "$INSTDIR\OddgravityWebsite.url"  "" "" "" "" "" "$(IDS_STARTMENU_HOMEPAGE_TIP)"
  !insertmacro MUI_STARTMENU_WRITE_END

  ; create uninstaller
  WriteUninstaller "$INSTDIR\uninstall.exe"

  ; add uninstall information to "Add/Remove Programs"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "DisplayName"     "${OG_PRODUCT_FULL}"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "UninstallString" "$INSTDIR\uninstall.exe"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "DisplayIcon"     "$INSTDIR\${OG_APP_FILENAME},0"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "Publisher"       "${OG_COMPANY}"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "URLInfoAbout"    "${OG_WEBSITE_URL}"
  WriteRegStr   HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "DisplayVersion"  "${OG_VERSION_BUILD}"
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "NoModify"        1
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}" "NoRepair"        1

SectionEnd

Section "Uninstall"

  ; define files to delete
  Delete "$INSTDIR\pc3.exe"
  Delete "$INSTDIR\cximage.dll"
  Delete "$INSTDIR\MediaInfo.dll"
  Delete "$INSTDIR\PC3_DE.chm"
  Delete "$INSTDIR\PC3_EN.chm"
  Delete "$INSTDIR\ReleaseNotes.htm"
  Delete "$INSTDIR\OddgravityWebsite.url"
  Delete "$INSTDIR\uninstall.exe"

  ; get start menu folder
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  ; delete start menu shortcuts
  Delete "$SMPROGRAMS\$MUI_TEMP\$(IDS_STARTMENU_APP_LNK).lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\$(IDS_STARTMENU_HELP_LNK).lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\$(IDS_STARTMENU_HOMEPAGE_LNK).lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\$(IDS_STARTMENU_RELNOTES_LNK).lnk"

  ; delete empty start menu parent directories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

  startMenuDeleteLoop:
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

    IfErrors startMenuDeleteLoopDone

    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  ; remove installation folder
  RMDir "$INSTDIR"

  ; delete desktop shortcut
  Delete "$DESKTOP\$(IDS_DESKTOP_SHORTCUT_LNK).lnk"

  ; clean up registry
  DeleteRegValue HKCU "Software\${OG_REGKEY}" "Installer Language"
  ; note: commented out because registry keys should be available for a newer version of the app
  ;DeleteRegKey /ifempty HKCU "Software\${OG_REGKEY}"

  ; delete uninstall information for "Add/Remove Programs"
  DeleteRegKey HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\${OG_PRODUCT_FULL}"

SectionEnd


;-------------------------------------------
; functions
;-------------------------------------------

Function .onInit
  ; elevate to admin level
  UAC::RunElevated
  StrCmp 1223 $0 UAC_ElevationAborted ; UAC dialog aborted by user?
  StrCmp 0 $0 0 UAC_Err ; error?
  StrCmp 1 $1 0 UAC_Success ; are we the real deal or just the wrapper?
  Quit
UAC_Err:
  MessageBox MB_OK|MB_ICONSTOP "Could not elevate process (errorcode $0). Installation will now be aborted."
  Abort
UAC_ElevationAborted:
  Abort
UAC_Success:

  ; display language selection dialog
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

Function .onInstSuccess
  UAC::Unload ; must be called, otherwise temp files will remain on disk
FunctionEnd

Function .onInstFailed
  UAC::Unload ; must be called, otherwise temp files will remain on disk
FunctionEnd

Function un.onInit
  ; get the stored language preference
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function CustomFinishRun
  ; start application with user privileges
  UAC::Exec '' '"$INSTDIR\${OG_APP_FILENAME}"' '' ''
FunctionEnd

Function CreateDesktopIcon
  CreateShortCut "$DESKTOP\$(IDS_DESKTOP_SHORTCUT_LNK).lnk" "$INSTDIR\${OG_APP_FILENAME}" "" "" "" "" "" "$(IDS_DESKTOP_SHORTCUT_TIP)"
FunctionEnd
