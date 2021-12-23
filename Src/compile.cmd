@echo off
setlocal

set ZLIB=%~dp0zlib-1.2.11

rem ===================================================================================================================

set BUILD=%~dp0_BUILD_
set OUT=%~dp0_PUBLISH_

if not exist %OUT%\bin mkdir %OUT%\bin
if not exist %OUT%\include mkdir %OUT%\include
if not exist %OUT%\lib mkdir %OUT%\lib

set PATH=%~dp0..\Toolchain\TDM-GCC\bin;%PATH%
rem C:\msys64\usr\bin;F:\__WINNT__\tmp\2\TDM-GCC\bin;%PATH%

rem ===================================================================================================================

copy /b %~dp0..\Toolchain\TDM-GCC\bin\libgcc_s_dw2-1.dll %OUT%\bin

rem ===================================================================================================================
rem  ZLIB
rem ===================================================================================================================

if not exist %BUILD%\zlib mkdir %BUILD%\zlib
if errorlevel 1 goto error
cd %BUILD%\zlib
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=mingw32-make -DCMAKE_BUILD_TYPE=Release %ZLIB%
if errorlevel 1 goto error

mingw32-make -j 4
if errorlevel 1 goto error
strip zlib1.dll
if errorlevel 1 goto error

copy /b zlib1.dll %OUT%\bin
if errorlevel 1 goto error
copy /b libzlib.dll.a %OUT%\lib
if errorlevel 1 goto error
copy /b libzlibstatic.a %OUT%\lib
if errorlevel 1 goto error
copy /b %ZLIB%\*.h %OUT%\include
if errorlevel 1 goto error
copy /b zconf.h %OUT%\include
if errorlevel 1 goto error

rem ===================================================================================================================
:error
