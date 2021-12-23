@echo off
setlocal

set ZLIB=%~dp0zlib-1.2.11
set OPENSSL=%~dp0openssl-1.1.1m

rem ==========================================================================

set PATH=%~dp0..\Toolchain\TDM-GCC\bin;%~dp0..\Toolchain\MSYS64\usr\bin;%PATH%

rem ==========================================================================

set BUILD=%~dp0_BUILD_
set OUT=%~dp0_PUBLISH_

if not exist %OUT%\bin mkdir %OUT%\bin
if errorlevel 1 goto error
if not exist %OUT%\include mkdir %OUT%\include
if errorlevel 1 goto error
if not exist %OUT%\lib mkdir %OUT%\lib
if errorlevel 1 goto error

rem ==========================================================================

if not "%1" == "" goto :%1

rem ==========================================================================

rem ========
rem  Common
rem ========

if exist %OUT%\bin\libgcc_s_dw2-1.dll goto haslibgcc
copy /b %~dp0..\Toolchain\TDM-GCC\bin\libgcc_s_dw2-1.dll %OUT%\bin
if errorlevel 1 goto error
strip %OUT%\bin\libgcc_s_dw2-1.dll
if errorlevel 1 goto error
:haslibgcc

rem ======
rem  ZLIB
rem ======
:zlib

if not exist %BUILD%\zlib mkdir %BUILD%\zlib
if errorlevel 1 goto error
cd %BUILD%\zlib
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=mingw32-make -DCMAKE_BUILD_TYPE=Release %ZLIB%
if errorlevel 1 goto error

mingw32-make -j 4
if errorlevel 1 goto error

cmake -E copy_if_different zlib.dll %OUT%\bin\zlib.dll
if errorlevel 1 goto error
cmake -E copy_if_different libzlib.dll.a %OUT%\lib
if errorlevel 1 goto error
cmake -E copy_if_different libzlibstatic.a %OUT%\lib
if errorlevel 1 goto error
for %%f in (%ZLIB%\*.h) do cmake -E copy_if_different %%f %OUT%\include
if errorlevel 1 goto error
cmake -E copy_if_different zconf.h %OUT%\include
if errorlevel 1 goto error

if not "%1" == "" goto next

rem =========
rem  OPENSSL
rem =========
:openssl

cd %OPENSSL%
if errorlevel 1 goto error

perl Configure mingw ^
    no-async ^
    no-sse2 ^
    no-external-tests ^
    no-tests ^
    zlib ^
    --openssldir=C:\MinGW ^
    --with-zlib-include=%OUT%\include ^
    --with-zlib-lib=%OUT%\lib ^
    -fno-ident ^
    -fno-stack-protector ^
    -fno-unwind-tables ^
    -fno-asynchronous-unwind-tables ^
    -Wl,--build-id=none ^
    -Wl,-s
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not exist %OUT%\include\openssl mkdir %OUT%\include\openssl
if errorlevel 1 goto error

cmake -E copy_if_different libcrypto.a %OUT%\lib
if errorlevel 1 goto error
cmake -E copy_if_different libcrypto.dll.a %OUT%\lib
if errorlevel 1 goto error
cmake -E copy_if_different libeay11.dll %OUT%\bin
if errorlevel 1 goto error
cmake -E copy_if_different libssl.a %OUT%\lib
if errorlevel 1 goto error
cmake -E copy_if_different libssl.dll.a %OUT%\lib
if errorlevel 1 goto error
cmake -E copy_if_different ssleay11.dll %OUT%\bin
if errorlevel 1 goto error
for %%f in (include\openssl\*.h) do cmake -E copy_if_different %%f %OUT%\include\openssl
if errorlevel 1 goto error

if not "%1" == "" goto next

rem ==========================================================================

:done
echo DONE!
exit

:next
shift
if not "%1" == "" goto :%1
goto done

:error
echo ERROR!
