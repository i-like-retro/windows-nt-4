
Var MUI_TEMP
Var STARTMENU_FOLDER
Var START_MENU
Var DO_NOT_ADD_TO_PATH
Var ADD_TO_PATH_ALL_USERS
Var ADD_TO_PATH_CURRENT_USER

!include "MUI.nsh"

Name "PuTTY 0.76"
InstallDir "$PROGRAMFILES\PuTTY"

SetCompressor lzma
RequestExecutionLevel admin

OutFile "..\..\..\CD\SOFTWARE\NETWORK\PUTTY076.EXE"

!define MUI_ICON "..\..\putty-0.76\icons\puttyins.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\..\putty-0.76\windows\msidialog.bmp"
!define MUI_HEADERIMAGE_BITMAP "..\..\putty-0.76\windows\msibanner.bmp"
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE
!define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_ABORTWARNING

!include "..\path.nsh"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\putty-0.76\LICENCE"
!insertmacro MUI_PAGE_DIRECTORY
Page custom InstallOptionsPage

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\PuTTY"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

ReserveFile "setup.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

Function .onInit
    StrCpy $INSTDIR "$PROGRAMFILES\PuTTY"
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "setup.ini"
FunctionEnd

Function InstallOptionsPage
    !insertmacro MUI_HEADER_TEXT "Install Options" "Choose options for installing PuTTY"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "setup.ini"
FunctionEnd

Function ConditionalAddToRegisty
    Pop $0
    Pop $1
    StrCmp "$0" "" ConditionalAddToRegisty_EmptyString
        WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY" "$1" "$0"
        DetailPrint "Set install registry entry: '$1' to '$0'"
  ConditionalAddToRegisty_EmptyString:
FunctionEnd

Section "-Main"
    WriteRegStr SHCTX "Software\PuTTY" "" $INSTDIR

    SetOutPath "$INSTDIR"
    File "..\..\putty-0.76\windows\pageant.exe"
    File "..\..\putty-0.76\windows\plink.exe"
    File "..\..\putty-0.76\windows\pscp.exe"
    File "..\..\putty-0.76\windows\psftp.exe"
    File "..\..\putty-0.76\windows\putty.exe"
    File "..\..\putty-0.76\windows\puttygen.exe"
    File /oname=README.txt "..\..\putty-0.76\windows\README-msi.txt"
    File /oname=LICENCE.txt "..\..\putty-0.76\LICENCE"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
    Push "DisplayName"
    Push "PuTTY"
    Call ConditionalAddToRegisty
    Push "UninstallString"
    Push "$INSTDIR\uninst.exe"
    Call ConditionalAddToRegisty
    Push "NoRepair"
    Push "1"
    Call ConditionalAddToRegisty
    Push "NoModify"
    Push "1"
    Call ConditionalAddToRegisty

    ; Optional registration
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Pageant.lnk" "$INSTDIR\pageant.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\PSFTP.lnk" "$INSTDIR\psftp.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\PuTTY.lnk" "$INSTDIR\putty.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\PuTTYgen.lnk" "$INSTDIR\puttygen.exe"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninst.exe"

    CreateShortCut "$DESKTOP\PuTTY.lnk" "$INSTDIR\putty.exe"

    ; Create associations
    WriteRegStr HKCR ".ppk" "" "PuTTY Key"
    WriteRegStr HKCR "PuTTY Key" "" "PuTTY Key"
    WriteRegStr HKCR "PuTTY Key\shell" "" "open"
    WriteRegStr HKCR "PuTTY Key\DefaultIcon" "" "$INSTDIR\pageant.exe,0"
    WriteRegStr HKCR "PuTTY Key\shell\open\command" "" '"$INSTDIR\pageant.exe" "%1"'
    WriteRegStr HKCR "PuTTY Key\shell\edit" "" "Edit"
    WriteRegStr HKCR "PuTTY Key\shell\edit\command" "" '"$INSTDIR\puttygen.exe" "%1"'

    ;Read a value from an InstallOptions INI file
    !insertmacro MUI_INSTALLOPTIONS_READ $ADD_TO_PATH_ALL_USERS "setup.ini" "Field 2" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $ADD_TO_PATH_CURRENT_USER "setup.ini" "Field 3" "State"
    !insertmacro MUI_INSTALLOPTIONS_READ $DO_NOT_ADD_TO_PATH "setup.ini" "Field 4" "State"

    ; Write special uninstall registry entries
    Push "StartMenu"
    Push "$STARTMENU_FOLDER"
    Call ConditionalAddToRegisty
    Push "DoNotAddToPath"
    Push "$DO_NOT_ADD_TO_PATH"
    Call ConditionalAddToRegisty
    Push "AddToPathAllUsers"
    Push "$ADD_TO_PATH_ALL_USERS"
    Call ConditionalAddToRegisty
    Push "AddToPathCurrentUser"
    Push "$ADD_TO_PATH_CURRENT_USER"
    Call ConditionalAddToRegisty

    !insertmacro MUI_STARTMENU_WRITE_END

    Push $INSTDIR
    StrCmp $DO_NOT_ADD_TO_PATH "1" doNotAddToPath 0
        Call AddToPath
  doNotAddToPath:

SectionEnd

Section "Uninstall"
    ReadRegStr $START_MENU SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY" "StartMenu"
    ReadRegStr $DO_NOT_ADD_TO_PATH SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY" "DoNotAddToPath"
    ReadRegStr $ADD_TO_PATH_ALL_USERS SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY" "AddToPathAllUsers"
    ReadRegStr $ADD_TO_PATH_CURRENT_USER SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY" "AddToPathCurrentUser"

    Delete "$INSTDIR\uninst.exe"
    Delete "$INSTDIR\LICENCE"
    Delete /REBOOTOK "$INSTDIR\pageant.exe"
    Delete "$INSTDIR\plink.exe"
    Delete "$INSTDIR\pscp.exe"
    Delete "$INSTDIR\psftp.exe"
    Delete "$INSTDIR\putty.exe"
    Delete "$INSTDIR\puttygen.exe"
    Delete "$INSTDIR\README.txt"
    Delete "$INSTDIR\LICENCE.txt"
    RMDir /REBOOTOK $INSTDIR

    DeleteRegKey HKCR ".ppk"
    DeleteRegKey HKCR "PuTTY Key"

    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\PuTTY"
    DeleteRegKey SHCTX "Software\PuTTY"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

    Delete "$SMPROGRAMS\$MUI_TEMP\Pageant.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\PSFTP.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\PuTTY.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\PuTTYgen.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
    Delete "$DESKTOP\PuTTY.lnk"

    ;Delete empty start menu parent directories
    StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
  startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    IfErrors startMenuDeleteLoopDone
    StrCmp "$MUI_TEMP" "$SMPROGRAMS" startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

    ; If the user changed the shortcut, then uninstall may not work. This should
    ; try to fix it.
    StrCpy $MUI_TEMP "$START_MENU"
    Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
    ;Delete empty start menu parent directories
    StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
  secondStartMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    IfErrors secondStartMenuDeleteLoopDone
    StrCmp "$MUI_TEMP" "$SMPROGRAMS" secondStartMenuDeleteLoopDone secondStartMenuDeleteLoop
  secondStartMenuDeleteLoopDone:

    DeleteRegKey /ifempty SHCTX "Software\PuTTY"

    Push $INSTDIR
    StrCmp $DO_NOT_ADD_TO_PATH_ "1" doNotRemoveFromPath 0
        Call un.RemoveFromPath
  doNotRemoveFromPath:

SectionEnd
