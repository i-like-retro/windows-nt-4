@echo off

if exist %~dp07Z2106.exe del %~dp07Z2106.exe

cd %~dp0CPP\7zip\UI\Explorer
if errorlevel 1 goto exit
msdev Explorer.dsw /MAKE "Explorer - Win32 ReleaseU"
if errorlevel 1 goto exit
if exist %~dp0Build\7-zip.dll del %~dp0Build\7-zip.dll
copy /b ReleaseU\7-Zip.dll %~dp0Build\7-zip.dll
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\Bundles\Format7zF
if errorlevel 1 goto exit
msdev Format7z.dsw /MAKE "7z - Win32 Release"
if errorlevel 1 goto exit
if exist %~dp0Build\7z.dll del %~dp0Build\7z.dll
copy /b Release\7z.dll %~dp0Build\7z.dll
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\UI\Console
if errorlevel 1 goto exit
msdev Console.dsw /MAKE "Console - Win32 ReleaseU"
if errorlevel 1 goto exit
if exist %~dp0Build\7z.exe del %~dp0Build\7z.exe
copy /b ReleaseU\7z.exe %~dp0Build\7z.exe
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\Bundles\SFXWin
if errorlevel 1 goto exit
msdev SFXWin.dsw /MAKE "SFXWin - Win32 Release"
if errorlevel 1 goto exit
if exist %~dp0Build\7z.sfx del %~dp0Build\7z.sfx
copy /b Release\7z.sfx %~dp0Build\7z.sfx
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\Bundles\SFXCon
if errorlevel 1 goto exit
msdev SFXCon.dsw /MAKE "SFXCon - Win32 Release"
if errorlevel 1 goto exit
if exist %~dp0Build\7zCon.sfx del %~dp0Build\7zCon.sfx
copy /b Release\7zCon.exe %~dp0Build\7zCon.sfx
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\Bundles\Fm
if errorlevel 1 goto exit
msdev FM.dsw /MAKE "FM - Win32 ReleaseU"
if errorlevel 1 goto exit
if exist %~dp0Build\7zFM.exe del %~dp0Build\7zFM.exe
copy /b ReleaseU\7zFM.exe %~dp0Build\7zFM.exe
if errorlevel 1 goto exit

cd %~dp0CPP\7zip\UI\GUI
if errorlevel 1 goto exit
msdev GUI.dsw /MAKE "GUI - Win32 ReleaseU"
if errorlevel 1 goto exit
if exist %~dp0Build\7zG.exe del %~dp0Build\7zG.exe
copy /b ReleaseU\7zg.exe %~dp0Build\7zG.exe
if errorlevel 1 goto exit

cd %~dp0C\Util\7zipUninstall
if errorlevel 1 goto exit
msdev 7zipUninstall.dsw /MAKE "7zipUninstall - Win32 Release"
if errorlevel 1 goto exit
if exist %~dp0Build\Uninstall.exe del %~dp0Build\Uninstall.exe
copy /b Release\Uninstall.exe %~dp0Build\Uninstall.exe
if errorlevel 1 goto exit

cd %~dp0C\Util\7zipInstall
if errorlevel 1 goto exit
msdev 7zipInstall.dsw /MAKE "7zipInstall - Win32 Release"
if errorlevel 1 goto exit

cd %~dp0Build
if errorlevel 1 goto exit

7z a -sfx..\C\Util\7zipInstall\Release\7zipInstall.exe %~dp07Z2106.exe ^
    Lang ^
    7-zip.chm ^
    7-zip.dll ^
    7z.dll ^
    7z.exe ^
    7z.sfx ^
    7zCon.sfx ^
    7zFM.exe ^
    7zG.exe ^
    descript.ion ^
    History.txt ^
    License.txt ^
    readme.txt ^
    Uninstall.exe
if errorlevel 1 goto exit

:exit
cd %~dp0
