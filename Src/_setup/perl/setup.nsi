
Var MUI_TEMP
Var STARTMENU_FOLDER
Var START_MENU
Var DO_NOT_ADD_TO_PATH
Var ADD_TO_PATH_ALL_USERS
Var ADD_TO_PATH_CURRENT_USER

!include "MUI.nsh"

Name "Perl 5.12.3.0"
InstallDir "$PROGRAMFILES\Perl"

SetCompressor lzma
RequestExecutionLevel admin

OutFile "..\..\..\CD\SOFTWARE\DEVEL\PERL5123.EXE"

!define MUI_ICON "perl.ico"
!define MUI_ABORTWARNING

!include "..\path.nsh"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\perl-5.12.3.0-bin\licenses\License.rtf"
!insertmacro MUI_PAGE_DIRECTORY
Page custom InstallOptionsPage

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Perl"
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
    StrCpy $INSTDIR "$PROGRAMFILES\Perl"
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "setup.ini"
FunctionEnd

Function InstallOptionsPage
    !insertmacro MUI_HEADER_TEXT "Install Options" "Choose options for installing Perl"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "setup.ini"
FunctionEnd

Function ConditionalAddToRegisty
    Pop $0
    Pop $1
    StrCmp "$0" "" ConditionalAddToRegisty_EmptyString
        WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl" "$1" "$0"
        DetailPrint "Set install registry entry: '$1' to '$0'"
  ConditionalAddToRegisty_EmptyString:
FunctionEnd

Section "-Main"
    WriteRegStr SHCTX "Software\Perl" "" $INSTDIR

    SetOutPath "$INSTDIR\perl"
    File /r "..\..\perl-5.12.3.0-bin\perl\*"
    SetOutPath "$INSTDIR\ppm"
    File /r "..\..\perl-5.12.3.0-bin\ppm\*"
    SetOutPath "$INSTDIR"
    File "..\..\perl-5.12.3.0-bin\DISTRIBUTIONS.txt"
    File "..\..\perl-5.12.3.0-bin\README.txt"
    File "..\..\perl-5.12.3.0-bin\README.portable.txt"
    File "..\..\perl-5.12.3.0-bin\portable.perl"
    File "..\..\perl-5.12.3.0-bin\portableshell.bat"
    CreateDirectory "$INSTDIR\cpan\sources"
    CreateDirectory "$INSTDIR\cpanplus"
    CreateDirectory "$INSTDIR\data"
    CreateDirectory "$INSTDIR\perl\site\bin"
    CreateDirectory "$INSTDIR\win32"
    CreateDirectory "$INSTDIR\c\include"
    CreateDirectory "$INSTDIR\c\lib"
    SetOutPath "$INSTDIR\c\bin"
    File "..\..\perl-5.12.3.0-bin\c\bin\dmake.exe"
    File "..\..\perl-5.12.3.0-bin\c\bin\patch.exe"
    SetOutPath "$INSTDIR\c\startup"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\config.mk"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\local.mk"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\startup.mk"
    SetOutPath "$INSTDIR\c\startup\winnt"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\winnt\macros.mk"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\winnt\recipes.mk"
    SetOutPath "$INSTDIR\c\startup\winnt\mingw"
    File "..\..\perl-5.12.3.0-bin\c\bin\startup\winnt\mingw\macros.mk"
    SetOutPath "$INSTDIR\perl\vendor\lib"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\FromCPAN"
    File "..\..\_build\git\_INSTALL_\share\perl5\FromCPAN\Error.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\FromCPAN\Mail"
    File "..\..\_build\git\_INSTALL_\share\perl5\FromCPAN\Mail\Address.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\Git"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\I18N.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\IndexInfo.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\LoadCPAN.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\Packet.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\Git\LoadCPAN"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\LoadCPAN\Error.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\Git\LoadCPAN\Mail"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\LoadCPAN\Mail\Address.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\Git\SVN"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Editor.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Fetcher.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\GlobSpec.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Log.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Migration.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Prompt.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Ra.pm"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Utils.pm"
    SetOutPath "$INSTDIR\perl\vendor\lib\Git\SVN\Memoize"
    File "..\..\_build\git\_INSTALL_\share\perl5\Git\SVN\Memoize\YAML.pm"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
    Push "DisplayName"
    Push "Perl"
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
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Perl Shell.lnk" "$INSTDIR\portableshell.bat" "" "$INSTDIR\perl\bin\perl.exe" 0
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninst.exe"

    ; Create associations
    WriteRegStr HKCR ".pl" "" "Perl Script"
    WriteRegStr HKCR "Perl Script" "" "Perl Script"
    WriteRegStr HKCR "Perl Script\shell" "" "open"
    WriteRegStr HKCR "Perl Script\DefaultIcon" "" "$INSTDIR\perl\bin\perl.exe,0"
    WriteRegStr HKCR "Perl Script\shell\open\command" "" '"$INSTDIR\perl\bin\perl.exe" "%1"'

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

    Push $INSTDIR\perl\site\bin
    StrCmp $DO_NOT_ADD_TO_PATH "1" doNotAddToPath1 0
        Call AddToPath
  doNotAddToPath1:

    Push $INSTDIR\perl\bin
    StrCmp $DO_NOT_ADD_TO_PATH "1" doNotAddToPath2 0
        Call AddToPath
  doNotAddToPath2:

SectionEnd

Section "Uninstall"
    ReadRegStr $START_MENU SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl" "StartMenu"
    ReadRegStr $DO_NOT_ADD_TO_PATH SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl" "DoNotAddToPath"
    ReadRegStr $ADD_TO_PATH_ALL_USERS SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl" "AddToPathAllUsers"
    ReadRegStr $ADD_TO_PATH_CURRENT_USER SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl" "AddToPathCurrentUser"

    Delete "$INSTDIR\uninst.exe"
    RMDir /R "$INSTDIR"

    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Perl"
    DeleteRegKey SHCTX "Software\Perl"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

    Delete "$SMPROGRAMS\$MUI_TEMP\Perl Shell.lnk"
    Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"

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

    DeleteRegKey /ifempty SHCTX "Software\Perl"

    Push $INSTDIR\perl\site\bin
    StrCmp $DO_NOT_ADD_TO_PATH_ "1" doNotRemoveFromPath1 0
        Call un.RemoveFromPath
  doNotRemoveFromPath1:

    Push $INSTDIR\perl\bin
    StrCmp $DO_NOT_ADD_TO_PATH_ "1" doNotRemoveFromPath2 0
        Call un.RemoveFromPath
  doNotRemoveFromPath2:

SectionEnd
