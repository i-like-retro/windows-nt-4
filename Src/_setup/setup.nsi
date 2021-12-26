
Var MUI_TEMP
Var STARTMENU_FOLDER
Var START_MENU
Var DO_NOT_ADD_TO_PATH
Var ADD_TO_PATH_ALL_USERS
Var ADD_TO_PATH_CURRENT_USER

!include "MUI.nsh"

Name "GNU for Windows NT"
InstallDir "$PROGRAMFILES\GNU"

SetCompressor lzma
RequestExecutionLevel admin

OutFile "..\..\CD\SOFTWARE\GNU-NT4.EXE"

!include "Sections.nsh"

!define MUI_ICON "gnu.ico"
!define MUI_HEADERIMAGE_BITMAP "logo.bmp"
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_HEADERIMAGE
!define MUI_ABORTWARNING

!verbose 3
!include "WinMessages.NSH"
!verbose 4

!include "path.nsh"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "licenses.txt"
!insertmacro MUI_PAGE_COMPONENTS
Page custom InstallOptionsPage
;!insertmacro MUI_PAGE_DIRECTORY

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\GNU-NT4"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

ReserveFile "setup.ini"
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

Function .onInit
    StrCpy $INSTDIR "$PROGRAMFILES\GNU"
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "setup.ini"
    Call .onSelChange
FunctionEnd

Function InstallOptionsPage
    !insertmacro MUI_HEADER_TEXT "Install Options" "Choose options for installing GNU for Windows NT"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "setup.ini"
FunctionEnd

Function ConditionalAddToRegisty
    Pop $0
    Pop $1
    StrCmp "$0" "" ConditionalAddToRegisty_EmptyString
        WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4" "$1" "$0"
        DetailPrint "Set install registry entry: '$1' to '$0'"
  ConditionalAddToRegisty_EmptyString:
FunctionEnd

Section "-Main"
    SetOutPath "$INSTDIR"
    WriteRegStr SHCTX "Software\GNU-NT4" "" $INSTDIR

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
    Push "DisplayName"
    Push "GNU for Windows NT"
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
    Push "DisplayIcon"
    Push "$INSTDIR\gnu.ico"
    Call ConditionalAddToRegisty
    Push "HelpLink"
    Push "https://github.com/i-like-retro/windows-nt-4"
    Call ConditionalAddToRegisty
    Push "URLInfoAbout"
    Push "https://github.com/i-like-retro/windows-nt-4"
    Call ConditionalAddToRegisty
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\uninst.exe"

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

    Push $INSTDIR\bin
    StrCmp "ON" "ON" 0 doNotAddToPath
    StrCmp $DO_NOT_ADD_TO_PATH "1" doNotAddToPath 0
        Call AddToPath
  doNotAddToPath:
SectionEnd

InstType "User"
InstType "Programmer"

Section "Base"
    SectionIn 1 2 RO
    SetOutPath "$INSTDIR"
    File "gnu.ico"
    File "licenses.txt"
    SetOutPath "$INSTDIR\bin"
    File "..\_build\libgcc_s_dw2-1.dll"
SectionEnd

SectionGroup /e "!Core"
    Section "curl-7.80.0" CURL
        SectionIn 1 2
        SetOutPath "$INSTDIR\bin"
        File "..\_build\curl\src\curl.exe"
        File "..\_build\curl\lib\libcurl.dll"
        File /oname=curl-ca.crt "..\cacert\cacert.pem"
    SectionEnd
    Section "curl-7.80.0-devel" CURL_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libcurl.a "..\_build\curl\lib\libcurl.dll.a"
        File "..\_build\curl\lib\libcurl.def"
        SetOutPath "$INSTDIR\include\curl"
        File "..\curl-7.80.0\include\curl\*.h"
    SectionEnd
    Section "libexpat-2.4.2" EXPAT
        SectionIn 2
        SetOutPath "$INSTDIR\bin"
        File "..\_build\expat\libexpat.dll"
    SectionEnd
    Section "libexpat-2.4.2-devel" EXPAT_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libexpat.a "..\_build\expat\libexpat.dll.a"
        File "..\_build\expat\libexpat.def"
        SetOutPath "$INSTDIR\include"
        File "..\_build\expat\expat_config.h"
        File "..\expat-2.4.2\lib\expat.h"
        File "..\expat-2.4.2\lib\expat_external.h"
    SectionEnd
    Section "libncurses-6.3" NCURSES
        SectionIn 2
        SetOutPath "$INSTDIR\bin"
        File "..\ncurses-6.3\lib\libform6.dll"
        File "..\ncurses-6.3\lib\libmenu6.dll"
        File "..\ncurses-6.3\lib\libpane6.dll"
        File "..\ncurses-6.3\lib\ncurses6.dll"
    SectionEnd
    Section "libncurses-6.3-devel" NCURSES_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libncurses.a "..\ncurses-6.3\lib\ncurses.dll.a"
        File /oname=libncurses-static.a "..\ncurses-6.3\lib\libncurses.a"
        File /oname=libform.a "..\ncurses-6.3\lib\libform.dll.a"
        File /oname=libform-static.a "..\ncurses-6.3\lib\libform.a"
        File /oname=libmenu.a "..\ncurses-6.3\lib\libmenu.dll.a"
        File /oname=libmenu-static.a "..\ncurses-6.3\lib\libmenu.a"
        File /oname=libpanel.a "..\ncurses-6.3\lib\libpane.dll.a"
        File /oname=libpanel-static.a "..\ncurses-6.3\lib\libpane.a"
        File /oname=libncurses.def "..\ncurses-6.3\lib\ncurses.def"
        File /oname=libpanel.def "..\ncurses-6.3\lib\libpane.def"
        File "..\ncurses-6.3\lib\libform.def"
        File "..\ncurses-6.3\lib\libmenu.def"
        SetOutPath "$INSTDIR\include"
        File "..\ncurses-6.3\include\curses.h"
        File /oname=ncurses.h "..\ncurses-6.3\include\curses.h"
        File "..\ncurses-6.3\include\unctrl.h"
        File "..\ncurses-6.3\include\ncurses_dll.h"
        File "..\ncurses-6.3\include\term.h"
        File "..\ncurses-6.3\include\termcap.h"
        File "..\ncurses-6.3\include\ncurses_mingw.h"
        File "..\ncurses-6.3\include\nc_mingw.h"
        File "..\ncurses-6.3\menu\eti.h"
        File "..\ncurses-6.3\menu\menu.h"
        File "..\ncurses-6.3\panel\panel.h"
        File "..\ncurses-6.3\form\form.h"
    SectionEnd
    Section "libssh2-1.10.0" LIBSSH2
        SectionIn 1 2
        SetOutPath "$INSTDIR\bin"
        File "..\_build\libssh2\src\libssh2.dll"
    SectionEnd
    Section "libssh2-1.10.0-devel" LIBSSH2_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libssh2.a "..\_build\libssh2\src\liblibssh2.dll.a"
        File "..\_build\libssh2\src\libssh2.def"
        SetOutPath "$INSTDIR\include"
        File "..\libssh2-1.10.0\include\*.h"
    SectionEnd
    Section "openssl-1.1.1m" OPENSSL
        SectionIn 1 2
        SetOutPath "$INSTDIR\bin"
        File "..\openssl-1.1.1m\libeay11.dll"
        File "..\openssl-1.1.1m\ssleay11.dll"
        File "..\openssl-1.1.1m\apps\openssl.exe"
        SetOutPath "$INSTDIR\share\openssl.1_1"
        File "..\openssl-1.1.1m\apps\openssl.cnf"
        File "..\openssl-1.1.1m\apps\ct_log_list.cnf"
        SetOutPath "$INSTDIR\share\openssl.1_1\engines"
        File "..\openssl-1.1.1m\engines\capi.dll"
        File "..\openssl-1.1.1m\engines\dasync.dll"
        File "..\openssl-1.1.1m\engines\padlock.dll"
    SectionEnd
    Section "openssl-1.1.1m-devel" OPENSSL_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libcrypto-static.a "..\openssl-1.1.1m\libcrypto.a"
        File /oname=libcrypto.a "..\openssl-1.1.1m\libcrypto.dll.a"
        File /oname=libssl-static.a "..\openssl-1.1.1m\libssl.a"
        File /oname=libssl.a "..\openssl-1.1.1m\libssl.dll.a"
        SetOutPath "$INSTDIR\include\openssl"
        File /x "__DECC_*.h" "..\openssl-1.1.1m\include\openssl\*.h"
    SectionEnd
    Section "zlib-1.2.11" ZLIB
        SectionIn 1 2
        SetOutPath "$INSTDIR\bin"
        File "..\_build\zlib\zlib.dll"
    SectionEnd
    Section "zlib-1.2.11-devel" ZLIB_DEVEL
        SectionIn 2
        SetOutPath "$INSTDIR\lib"
        File /oname=libz.a "..\_build\zlib\libzlib.dll.a"
        File /oname=libz-static.a "..\_build\zlib\libzlibstatic.a"
        File /oname=libz.def "..\_build\zlib\zlib.def"
        SetOutPath "$INSTDIR\include"
        File "..\_build\zlib\zconf.h"
        File "..\zlib-1.2.11\crc32.h"
        File "..\zlib-1.2.11\deflate.h"
        File "..\zlib-1.2.11\gzguts.h"
        File "..\zlib-1.2.11\inffast.h"
        File "..\zlib-1.2.11\inffixed.h"
        File "..\zlib-1.2.11\inflate.h"
        File "..\zlib-1.2.11\inftrees.h"
        File "..\zlib-1.2.11\trees.h"
        File "..\zlib-1.2.11\zlib.h"
        File "..\zlib-1.2.11\zutil.h"
    SectionEnd
SectionGroupEnd

Var CURL
Var CURL_DEVEL
Var EXPAT
Var EXPAT_DEVEL
Var NCURSES
Var NCURSES_DEVEL
Var LIBSSH2
Var LIBSSH2_DEVEL
Var OPENSSL
Var OPENSSL_DEVEL
Var ZLIB
Var ZLIB_DEVEL

!macro Depends what alt dep
    !define UID ${__LINE__}
    ${If} ${SectionIsSelected} ${what}
        StrCpy ${dep} "YES"
    ${EndIf}
    StrCmp ${alt} "YES" isSet${UID} isNotSet${UID}
  isSet${UID}:
    StrCpy ${dep} "YES"
  isNotSet${UID}:
    !undef UID
!macroend

!macro MaybeNeeded what dep
    !define UID ${__LINE__}
    StrCmp ${dep} "YES" isNeeded${UID} isNotNeeded${UID}
  isNeeded${UID}:
    !insertmacro SelectSection ${what}
    !insertmacro SetSectionFlag ${what} ${SF_RO}
    Goto neededDone${UID}
  isNotNeeded${UID}:
    !insertmacro ClearSectionFlag ${what} ${SF_RO}
  neededDone${UID}:
    !undef UID
!macroend

Function .onSelChange
    StrCpy $CURL                "NO"
    StrCpy $CURL_DEVEL          "NO"
    StrCpy $EXPAT               "NO"
    StrCpy $EXPAT_DEVEL         "NO"
    StrCpy $NCURSES             "NO"
    StrCpy $NCURSES_DEVEL       "NO"
    StrCpy $LIBSSH2             "NO"
    StrCpy $LIBSSH2_DEVEL       "NO"
    StrCpy $OPENSSL             "NO"
    StrCpy $OPENSSL_DEVEL       "NO"
    StrCpy $ZLIB                "NO"
    StrCpy $ZLIB_DEVEL          "NO"

    !insertmacro Depends        ${CURL_DEVEL}       $CURL_DEVEL         $CURL
    !insertmacro Depends        ${EXPAT_DEVEL}      $EXPAT_DEVEL        $EXPAT
    !insertmacro Depends        ${NCURSES_DEVEL}    $NCURSES_DEVEL      $NCURSES
    !insertmacro Depends        ${LIBSSH2_DEVEL}    $LIBSSH2_DEVEL      $LIBSSH2
    !insertmacro Depends        ${OPENSSL_DEVEL}    $OPENSSL_DEVEL      $OPENSSL
    !insertmacro Depends        ${ZLIB_DEVEL}       $ZLIB_DEVEL         $ZLIB
    !insertmacro Depends        ${CURL}             $CURL               $LIBSSH2
    !insertmacro Depends        ${CURL}             $CURL               $OPENSSL
    !insertmacro Depends        ${CURL}             $CURL               $ZLIB
    !insertmacro Depends        ${LIBSSH2}          $LIBSSH2            $OPENSSL
    !insertmacro Depends        ${LIBSSH2}          $LIBSSH2            $ZLIB
    !insertmacro Depends        ${OPENSSL}          $OPENSSL            $ZLIB

    !insertmacro MaybeNeeded    ${CURL}             $CURL
    !insertmacro MaybeNeeded    ${CURL_DEVEL}       $CURL_DEVEL
    !insertmacro MaybeNeeded    ${EXPAT}            $EXPAT
    !insertmacro MaybeNeeded    ${EXPAT_DEVEL}      $EXPAT_DEVEL
    !insertmacro MaybeNeeded    ${NCURSES}          $NCURSES
    !insertmacro MaybeNeeded    ${NCURSES_DEVEL}    $NCURSES_DEVEL
    !insertmacro MaybeNeeded    ${LIBSSH2}          $LIBSSH2
    !insertmacro MaybeNeeded    ${LIBSSH2_DEVEL}    $LIBSSH2_DEVEL
    !insertmacro MaybeNeeded    ${OPENSSL}          $OPENSSL
    !insertmacro MaybeNeeded    ${OPENSSL_DEVEL}    $OPENSSL_DEVEL
    !insertmacro MaybeNeeded    ${ZLIB}             $ZLIB
    !insertmacro MaybeNeeded    ${ZLIB_DEVEL}       $ZLIB_DEVEL
FunctionEnd

Section "Uninstall"
    ReadRegStr $START_MENU SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4" "StartMenu"
    ReadRegStr $DO_NOT_ADD_TO_PATH SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4" "DoNotAddToPath"
    ReadRegStr $ADD_TO_PATH_ALL_USERS SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4" "AddToPathAllUsers"
    ReadRegStr $ADD_TO_PATH_CURRENT_USER SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4" "AddToPathCurrentUser"

    RMDir /R "$INSTDIR"

    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\GNU-NT4"
    DeleteRegKey SHCTX "Software\GNU-NT4"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

    Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"

    ;Delete empty start menu parent diretories
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
    ;Delete empty start menu parent diretories
    StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
  secondStartMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    IfErrors secondStartMenuDeleteLoopDone
    StrCmp "$MUI_TEMP" "$SMPROGRAMS" secondStartMenuDeleteLoopDone secondStartMenuDeleteLoop
  secondStartMenuDeleteLoopDone:

    DeleteRegKey /ifempty SHCTX "Software\GNU-NT4"

    Push $INSTDIR\bin
    StrCmp $DO_NOT_ADD_TO_PATH_ "1" doNotRemoveFromPath 0
        Call un.RemoveFromPath
  doNotRemoveFromPath:
SectionEnd
