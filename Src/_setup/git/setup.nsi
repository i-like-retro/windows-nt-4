
Var MUI_TEMP
Var STARTMENU_FOLDER
Var START_MENU
Var DO_NOT_ADD_TO_PATH
Var ADD_TO_PATH_ALL_USERS
Var ADD_TO_PATH_CURRENT_USER

!include "MUI.nsh"

Name "Git 2.34.1"
InstallDir "$PROGRAMFILES\Git"

SetCompressor lzma
RequestExecutionLevel admin

OutFile "..\..\..\CD\SOFTWARE\DEVEL\GIT2341.EXE"

!define MUI_ABORTWARNING

!include "..\path.nsh"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\..\git-2.34.1\COPYING"
!insertmacro MUI_PAGE_DIRECTORY
Page custom InstallOptionsPage

!define MUI_STARTMENUPAGE_REGISTRY_ROOT "SHCTX"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Git"
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
    StrCpy $INSTDIR "$PROGRAMFILES\Git"
    !insertmacro MUI_INSTALLOPTIONS_EXTRACT "setup.ini"
FunctionEnd

Function InstallOptionsPage
    !insertmacro MUI_HEADER_TEXT "Install Options" "Choose options for installing Git"
    !insertmacro MUI_INSTALLOPTIONS_DISPLAY "setup.ini"
FunctionEnd

Function ConditionalAddToRegisty
    Pop $0
    Pop $1
    StrCmp "$0" "" ConditionalAddToRegisty_EmptyString
        WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git" "$1" "$0"
        DetailPrint "Set install registry entry: '$1' to '$0'"
  ConditionalAddToRegisty_EmptyString:
FunctionEnd

Section "-Main"
    WriteRegStr SHCTX "Software\Git" "" $INSTDIR

    SetOutPath "$INSTDIR"
    File /oname=COPYING.txt "..\..\git-2.34.1\COPYING"
    SetOutPath "$INSTDIR\bin"
    File "..\..\_build\git\_INSTALL_\bin\git.exe"
    File "..\..\_build\git\_INSTALL_\bin\git-cvsserver"
    File "..\..\_build\git\_INSTALL_\bin\git-receive-pack.exe"
    File "..\..\_build\git\_INSTALL_\bin\git-shell.exe"
    File "..\..\_build\git\_INSTALL_\bin\git-upload-archive.exe"
    File "..\..\_build\git\_INSTALL_\bin\git-upload-pack.exe"
    File "..\..\_build\git\git.dll"
    File "..\..\_ntcompat\ntcompat.dll"
    File "..\..\less-394-bin\bin\less.exe"
    File "..\..\less-394-bin\bin\pcre3.dll"
    SetOutPath "$INSTDIR\libexec\git-core"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-add--interactive"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-add.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-am.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-annotate.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-apply.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-archimport"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-archive.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-bisect"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-bisect--helper.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-blame.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-branch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-bugreport.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-bundle.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cat-file.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-check-attr.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-check-ignore.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-check-mailmap.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-check-ref-format.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-checkout--worker.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-checkout-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-checkout.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cherry-pick.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cherry.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-clean.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-clone.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-column.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-commit-graph.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-commit-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-commit.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-config.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-count-objects.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-credential-cache--daemon.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-credential-cache.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-credential-store.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-credential.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cvsexportcommit"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cvsimport"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-cvsserver"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-daemon.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-describe.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-diff-files.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-diff-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-diff-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-diff.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-difftool--helper"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-difftool.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-env--helper.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fast-export.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fast-import.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fetch-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fetch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-filter-branch"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fmt-merge-msg.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-for-each-ref.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-for-each-repo.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-format-patch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fsck-objects.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-fsck.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-gc.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-get-tar-commit-id.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-grep.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-hash-object.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-help.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-http-backend.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-http-fetch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-http-push.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-imap-send.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-index-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-init-db.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-init.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-instaweb"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-interpret-trailers.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-log.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-ls-files.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-ls-remote.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-ls-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mailinfo.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mailsplit.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-maintenance.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-base.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-file.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-octopus"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-one-file"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-ours.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-recursive.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-resolve"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-subtree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-merge.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mergetool"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mergetool--lib"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mktag.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mktree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-multi-pack-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-mv.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-name-rev.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-notes.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-p4"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-pack-objects.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-pack-redundant.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-pack-refs.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-patch-id.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-prune-packed.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-prune.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-pull.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-push.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-quiltimport"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-range-diff.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-read-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-rebase.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-receive-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-reflog.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-ext.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-fd.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-ftp.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-ftps.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-http.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote-https.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-remote.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-repack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-replace.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-request-pull"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-rerere.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-reset.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-restore.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-rev-list.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-rev-parse.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-revert.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-rm.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-send-email"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-send-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-sh-i18n"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-sh-i18n--envsubst.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-sh-setup"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-shell.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-shortlog.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-show-branch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-show-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-show-ref.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-show.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-sparse-checkout.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-stage.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-stash.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-status.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-stripspace.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-submodule"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-submodule--helper.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-svn"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-switch.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-symbolic-ref.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-tag.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-unpack-file.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-unpack-objects.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-update-index.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-update-ref.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-update-server-info.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-upload-archive.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-upload-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-var.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-verify-commit.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-verify-pack.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-verify-tag.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-web--browse"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-whatchanged.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-worktree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git-write-tree.exe"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\git.exe"
    SetOutPath "$INSTDIR\libexec\git-core\mergetools"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\araxis"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\bc"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\codecompare"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\deltawalker"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\diffmerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\diffuse"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\ecmerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\emerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\examdiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\guiffy"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\gvimdiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\kdiff3"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\kompare"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\meld"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\nvimdiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\opendiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\p4merge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\smerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\tkdiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\tortoisemerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\vimdiff"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\winmerge"
    File "..\..\_build\git\_INSTALL_\libexec\git-core\mergetools\xxdiff"
    SetOutPath "$INSTDIR\share"
    File /oname=rootca.crt "..\..\cacert\cacert.pem"
    SetOutPath "$INSTDIR\share\git-core\templates"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\description"
    CreateDirectory "$INSTDIR\share\git-core\templates\branches"
    SetOutPath "$INSTDIR\share\git-core\templates\hooks"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\applypatch-msg.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\commit-msg.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\fsmonitor-watchman.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\post-update.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-applypatch.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-commit.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-merge-commit.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-push.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-rebase.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\pre-receive.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\prepare-commit-msg.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\push-to-checkout.sample"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\hooks\update.sample"
    SetOutPath "$INSTDIR\share\git-core\templates\info"
    File "..\..\_build\git\_INSTALL_\share\git-core\templates\info\exclude"

    ;Create uninstaller
    WriteUninstaller "$INSTDIR\uninst.exe"
    Push "DisplayName"
    Push "Git"
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

    WriteRegStr HKCU "Environment" "LESSANSIENDCHARS" "|"
    WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "LESSANSIENDCHARS" "|"

    Push $INSTDIR\bin
    StrCmp "ON" "ON" 0 doNotAddToPath
    StrCmp $DO_NOT_ADD_TO_PATH "1" doNotAddToPath 0
        Call AddToPath
  doNotAddToPath:

SectionEnd

Section "Uninstall"
    ReadRegStr $START_MENU SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git" "StartMenu"
    ReadRegStr $DO_NOT_ADD_TO_PATH SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git" "DoNotAddToPath"
    ReadRegStr $ADD_TO_PATH_ALL_USERS SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git" "AddToPathAllUsers"
    ReadRegStr $ADD_TO_PATH_CURRENT_USER SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git" "AddToPathCurrentUser"

    Delete "$INSTDIR\uninst.exe"
    Delete "$INSTDIR\COPYING.txt"
    Delete "$INSTDIR\bin\git.exe"
    Delete "$INSTDIR\bin\git-cvsserver"
    Delete "$INSTDIR\bin\git-receive-pack.exe"
    Delete "$INSTDIR\bin\git-shell.exe"
    Delete "$INSTDIR\bin\git-upload-archive.exe"
    Delete "$INSTDIR\bin\git-upload-pack.exe"
    Delete "$INSTDIR\bin\git.dll"
    Delete "$INSTDIR\bin\ntcompat.dll"
    Delete "$INSTDIR\bin\less.exe"
    Delete "$INSTDIR\bin\pcre3.dll"
    RMDir "$INSTDIR\bin"
    Delete "$INSTDIR\libexec\git-core\git-add--interactive"
    Delete "$INSTDIR\libexec\git-core\git-add.exe"
    Delete "$INSTDIR\libexec\git-core\git-am.exe"
    Delete "$INSTDIR\libexec\git-core\git-annotate.exe"
    Delete "$INSTDIR\libexec\git-core\git-apply.exe"
    Delete "$INSTDIR\libexec\git-core\git-archimport"
    Delete "$INSTDIR\libexec\git-core\git-archive.exe"
    Delete "$INSTDIR\libexec\git-core\git-bisect"
    Delete "$INSTDIR\libexec\git-core\git-bisect--helper.exe"
    Delete "$INSTDIR\libexec\git-core\git-blame.exe"
    Delete "$INSTDIR\libexec\git-core\git-branch.exe"
    Delete "$INSTDIR\libexec\git-core\git-bugreport.exe"
    Delete "$INSTDIR\libexec\git-core\git-bundle.exe"
    Delete "$INSTDIR\libexec\git-core\git-cat-file.exe"
    Delete "$INSTDIR\libexec\git-core\git-check-attr.exe"
    Delete "$INSTDIR\libexec\git-core\git-check-ignore.exe"
    Delete "$INSTDIR\libexec\git-core\git-check-mailmap.exe"
    Delete "$INSTDIR\libexec\git-core\git-check-ref-format.exe"
    Delete "$INSTDIR\libexec\git-core\git-checkout--worker.exe"
    Delete "$INSTDIR\libexec\git-core\git-checkout-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-checkout.exe"
    Delete "$INSTDIR\libexec\git-core\git-cherry-pick.exe"
    Delete "$INSTDIR\libexec\git-core\git-cherry.exe"
    Delete "$INSTDIR\libexec\git-core\git-clean.exe"
    Delete "$INSTDIR\libexec\git-core\git-clone.exe"
    Delete "$INSTDIR\libexec\git-core\git-column.exe"
    Delete "$INSTDIR\libexec\git-core\git-commit-graph.exe"
    Delete "$INSTDIR\libexec\git-core\git-commit-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git-commit.exe"
    Delete "$INSTDIR\libexec\git-core\git-config.exe"
    Delete "$INSTDIR\libexec\git-core\git-count-objects.exe"
    Delete "$INSTDIR\libexec\git-core\git-credential-cache--daemon.exe"
    Delete "$INSTDIR\libexec\git-core\git-credential-cache.exe"
    Delete "$INSTDIR\libexec\git-core\git-credential-store.exe"
    Delete "$INSTDIR\libexec\git-core\git-credential.exe"
    Delete "$INSTDIR\libexec\git-core\git-cvsexportcommit"
    Delete "$INSTDIR\libexec\git-core\git-cvsimport"
    Delete "$INSTDIR\libexec\git-core\git-cvsserver"
    Delete "$INSTDIR\libexec\git-core\git-daemon.exe"
    Delete "$INSTDIR\libexec\git-core\git-describe.exe"
    Delete "$INSTDIR\libexec\git-core\git-diff-files.exe"
    Delete "$INSTDIR\libexec\git-core\git-diff-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-diff-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git-diff.exe"
    Delete "$INSTDIR\libexec\git-core\git-difftool--helper"
    Delete "$INSTDIR\libexec\git-core\git-difftool.exe"
    Delete "$INSTDIR\libexec\git-core\git-env--helper.exe"
    Delete "$INSTDIR\libexec\git-core\git-fast-export.exe"
    Delete "$INSTDIR\libexec\git-core\git-fast-import.exe"
    Delete "$INSTDIR\libexec\git-core\git-fetch-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-fetch.exe"
    Delete "$INSTDIR\libexec\git-core\git-filter-branch"
    Delete "$INSTDIR\libexec\git-core\git-fmt-merge-msg.exe"
    Delete "$INSTDIR\libexec\git-core\git-for-each-ref.exe"
    Delete "$INSTDIR\libexec\git-core\git-for-each-repo.exe"
    Delete "$INSTDIR\libexec\git-core\git-format-patch.exe"
    Delete "$INSTDIR\libexec\git-core\git-fsck-objects.exe"
    Delete "$INSTDIR\libexec\git-core\git-fsck.exe"
    Delete "$INSTDIR\libexec\git-core\git-gc.exe"
    Delete "$INSTDIR\libexec\git-core\git-get-tar-commit-id.exe"
    Delete "$INSTDIR\libexec\git-core\git-grep.exe"
    Delete "$INSTDIR\libexec\git-core\git-hash-object.exe"
    Delete "$INSTDIR\libexec\git-core\git-help.exe"
    Delete "$INSTDIR\libexec\git-core\git-http-backend.exe"
    Delete "$INSTDIR\libexec\git-core\git-http-fetch.exe"
    Delete "$INSTDIR\libexec\git-core\git-http-push.exe"
    Delete "$INSTDIR\libexec\git-core\git-imap-send.exe"
    Delete "$INSTDIR\libexec\git-core\git-index-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-init-db.exe"
    Delete "$INSTDIR\libexec\git-core\git-init.exe"
    Delete "$INSTDIR\libexec\git-core\git-instaweb"
    Delete "$INSTDIR\libexec\git-core\git-interpret-trailers.exe"
    Delete "$INSTDIR\libexec\git-core\git-log.exe"
    Delete "$INSTDIR\libexec\git-core\git-ls-files.exe"
    Delete "$INSTDIR\libexec\git-core\git-ls-remote.exe"
    Delete "$INSTDIR\libexec\git-core\git-ls-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git-mailinfo.exe"
    Delete "$INSTDIR\libexec\git-core\git-mailsplit.exe"
    Delete "$INSTDIR\libexec\git-core\git-maintenance.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-base.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-file.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-octopus"
    Delete "$INSTDIR\libexec\git-core\git-merge-one-file"
    Delete "$INSTDIR\libexec\git-core\git-merge-ours.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-recursive.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-resolve"
    Delete "$INSTDIR\libexec\git-core\git-merge-subtree.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git-merge.exe"
    Delete "$INSTDIR\libexec\git-core\git-mergetool"
    Delete "$INSTDIR\libexec\git-core\git-mergetool--lib"
    Delete "$INSTDIR\libexec\git-core\git-mktag.exe"
    Delete "$INSTDIR\libexec\git-core\git-mktree.exe"
    Delete "$INSTDIR\libexec\git-core\git-multi-pack-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-mv.exe"
    Delete "$INSTDIR\libexec\git-core\git-name-rev.exe"
    Delete "$INSTDIR\libexec\git-core\git-notes.exe"
    Delete "$INSTDIR\libexec\git-core\git-p4"
    Delete "$INSTDIR\libexec\git-core\git-pack-objects.exe"
    Delete "$INSTDIR\libexec\git-core\git-pack-redundant.exe"
    Delete "$INSTDIR\libexec\git-core\git-pack-refs.exe"
    Delete "$INSTDIR\libexec\git-core\git-patch-id.exe"
    Delete "$INSTDIR\libexec\git-core\git-prune-packed.exe"
    Delete "$INSTDIR\libexec\git-core\git-prune.exe"
    Delete "$INSTDIR\libexec\git-core\git-pull.exe"
    Delete "$INSTDIR\libexec\git-core\git-push.exe"
    Delete "$INSTDIR\libexec\git-core\git-quiltimport"
    Delete "$INSTDIR\libexec\git-core\git-range-diff.exe"
    Delete "$INSTDIR\libexec\git-core\git-read-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git-rebase.exe"
    Delete "$INSTDIR\libexec\git-core\git-receive-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-reflog.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-ext.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-fd.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-ftp.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-ftps.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-http.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote-https.exe"
    Delete "$INSTDIR\libexec\git-core\git-remote.exe"
    Delete "$INSTDIR\libexec\git-core\git-repack.exe"
    Delete "$INSTDIR\libexec\git-core\git-replace.exe"
    Delete "$INSTDIR\libexec\git-core\git-request-pull"
    Delete "$INSTDIR\libexec\git-core\git-rerere.exe"
    Delete "$INSTDIR\libexec\git-core\git-reset.exe"
    Delete "$INSTDIR\libexec\git-core\git-restore.exe"
    Delete "$INSTDIR\libexec\git-core\git-rev-list.exe"
    Delete "$INSTDIR\libexec\git-core\git-rev-parse.exe"
    Delete "$INSTDIR\libexec\git-core\git-revert.exe"
    Delete "$INSTDIR\libexec\git-core\git-rm.exe"
    Delete "$INSTDIR\libexec\git-core\git-send-email"
    Delete "$INSTDIR\libexec\git-core\git-send-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-sh-i18n"
    Delete "$INSTDIR\libexec\git-core\git-sh-i18n--envsubst.exe"
    Delete "$INSTDIR\libexec\git-core\git-sh-setup"
    Delete "$INSTDIR\libexec\git-core\git-shell.exe"
    Delete "$INSTDIR\libexec\git-core\git-shortlog.exe"
    Delete "$INSTDIR\libexec\git-core\git-show-branch.exe"
    Delete "$INSTDIR\libexec\git-core\git-show-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-show-ref.exe"
    Delete "$INSTDIR\libexec\git-core\git-show.exe"
    Delete "$INSTDIR\libexec\git-core\git-sparse-checkout.exe"
    Delete "$INSTDIR\libexec\git-core\git-stage.exe"
    Delete "$INSTDIR\libexec\git-core\git-stash.exe"
    Delete "$INSTDIR\libexec\git-core\git-status.exe"
    Delete "$INSTDIR\libexec\git-core\git-stripspace.exe"
    Delete "$INSTDIR\libexec\git-core\git-submodule"
    Delete "$INSTDIR\libexec\git-core\git-submodule--helper.exe"
    Delete "$INSTDIR\libexec\git-core\git-svn"
    Delete "$INSTDIR\libexec\git-core\git-switch.exe"
    Delete "$INSTDIR\libexec\git-core\git-symbolic-ref.exe"
    Delete "$INSTDIR\libexec\git-core\git-tag.exe"
    Delete "$INSTDIR\libexec\git-core\git-unpack-file.exe"
    Delete "$INSTDIR\libexec\git-core\git-unpack-objects.exe"
    Delete "$INSTDIR\libexec\git-core\git-update-index.exe"
    Delete "$INSTDIR\libexec\git-core\git-update-ref.exe"
    Delete "$INSTDIR\libexec\git-core\git-update-server-info.exe"
    Delete "$INSTDIR\libexec\git-core\git-upload-archive.exe"
    Delete "$INSTDIR\libexec\git-core\git-upload-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-var.exe"
    Delete "$INSTDIR\libexec\git-core\git-verify-commit.exe"
    Delete "$INSTDIR\libexec\git-core\git-verify-pack.exe"
    Delete "$INSTDIR\libexec\git-core\git-verify-tag.exe"
    Delete "$INSTDIR\libexec\git-core\git-web--browse"
    Delete "$INSTDIR\libexec\git-core\git-whatchanged.exe"
    Delete "$INSTDIR\libexec\git-core\git-worktree.exe"
    Delete "$INSTDIR\libexec\git-core\git-write-tree.exe"
    Delete "$INSTDIR\libexec\git-core\git.exe"
    Delete "$INSTDIR\libexec\git-core\mergetools\araxis"
    Delete "$INSTDIR\libexec\git-core\mergetools\bc"
    Delete "$INSTDIR\libexec\git-core\mergetools\codecompare"
    Delete "$INSTDIR\libexec\git-core\mergetools\deltawalker"
    Delete "$INSTDIR\libexec\git-core\mergetools\diffmerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\diffuse"
    Delete "$INSTDIR\libexec\git-core\mergetools\ecmerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\emerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\examdiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\guiffy"
    Delete "$INSTDIR\libexec\git-core\mergetools\gvimdiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\kdiff3"
    Delete "$INSTDIR\libexec\git-core\mergetools\kompare"
    Delete "$INSTDIR\libexec\git-core\mergetools\meld"
    Delete "$INSTDIR\libexec\git-core\mergetools\nvimdiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\opendiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\p4merge"
    Delete "$INSTDIR\libexec\git-core\mergetools\smerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\tkdiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\tortoisemerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\vimdiff"
    Delete "$INSTDIR\libexec\git-core\mergetools\winmerge"
    Delete "$INSTDIR\libexec\git-core\mergetools\xxdiff"
    RMDir "$INSTDIR\libexec\git-core\mergetools"
    RMDir "$INSTDIR\libexec\git-core"
    RMDir "$INSTDIR\libexec"
    Delete "$INSTDIR\share\rootca.crt"
    Delete "$INSTDIR\share\git-core\templates\description"
    RMDir "$INSTDIR\share\git-core\templates\branches"
    Delete "$INSTDIR\share\git-core\templates\hooks\applypatch-msg.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\commit-msg.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\fsmonitor-watchman.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\post-update.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-applypatch.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-commit.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-merge-commit.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-push.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-rebase.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\pre-receive.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\prepare-commit-msg.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\push-to-checkout.sample"
    Delete "$INSTDIR\share\git-core\templates\hooks\update.sample"
    RMDir "$INSTDIR\share\git-core\templates\hooks"
    Delete "$INSTDIR\share\git-core\templates\info\exclude"
    RMDir "$INSTDIR\share\git-core\templates\info"
    RMDir "$INSTDIR\share\git-core\templates"
    RMDir "$INSTDIR\share\git-core"
    RMDir "$INSTDIR\share"
    RMDir "$INSTDIR"

    DeleteRegValue HKCU "Environment" "LESSANSIENDCHARS"
    DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "LESSANSIENDCHARS"
    DeleteRegKey SHCTX "Software\Microsoft\Windows\CurrentVersion\Uninstall\Git"
    DeleteRegKey SHCTX "Software\Git"

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

    DeleteRegKey /ifempty SHCTX "Software\Git"

    Push $INSTDIR\bin
    StrCmp $DO_NOT_ADD_TO_PATH_ "1" doNotRemoveFromPath 0
        Call un.RemoveFromPath
  doNotRemoveFromPath:
SectionEnd