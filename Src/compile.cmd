@echo off
setlocal

set ZLIB=%~dp0zlib-1.2.11
set OPENSSL=%~dp0openssl-1.1.1m
set LIBSSH2=%~dp0libssh2-1.10.0
set CURL=%~dp0curl-7.80.0

rem ==========================================================================

set TOOLCHAIN=%~dp0..\Toolchain
set BUILD=%~dp0_build

set PATH=%TOOLCHAIN%\TDM-GCC\bin;%TOOLCHAIN%\MSYS64\usr\bin;%TOOLCHAIN%\NSIS;%PATH%

rem ==========================================================================

if not exist %BUILD% mkdir %BUILD%
if errorlevel 1 goto error

if not "%1" == "" goto :%1

rem ==========================================================================

rem ========
rem  Common
rem ========

if exist %BUILD%\libgcc_s_dw2-1.dll goto haslibgcc
copy /b %~dp0..\Toolchain\TDM-GCC\bin\libgcc_s_dw2-1.dll %BUILD%
if errorlevel 1 goto error
strip %BUILD%\libgcc_s_dw2-1.dll
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

cmake -G "MinGW Makefiles" ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DCMAKE_BUILD_TYPE=Release ^
    %ZLIB%
if errorlevel 1 goto error
mingw32-make -j 4
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
    "--openssldir=C:\Program Files\GNU" ^
    --with-zlib-include=%ZLIB% ^
    --with-zlib-lib=%BUILD%\zlib\libzlib.dll.a ^
    -I%BUILD%\zlib ^
    -L%BUILD%\zlib ^
    -fno-ident ^
    -fno-stack-protector ^
    -fno-unwind-tables ^
    -fno-asynchronous-unwind-tables ^
    -Wl,--build-id=none ^
    -Wl,-s
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not "%1" == "" goto next

rem =========
rem  LIBSSH2
rem =========
:libssh2

if not exist %BUILD%\libssh2 mkdir %BUILD%\libssh2
if errorlevel 1 goto error
cd %BUILD%\libssh2
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_SHARED_LIBS:BOOL=YES ^
    -DENABLE_ZLIB_COMPRESSION:BOOL=YES ^
    -DBUILD_EXAMPLES=OFF ^
    -DBUILD_TESTING=OFF ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DZLIB_INCLUDE_DIR:PATH=%ZLIB%;%BUILD%\zlib ^
    -DZLIB_LIBRARY:PATH=%BUILD%\zlib\libzlib.dll.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    -DOPENSSL_INCLUDE_DIR:PATH=%OPENSSL%\include ^
    -DOPENSSL_CRYPTO_LIBRARY:PATH=%OPENSSL%\libcrypto.dll.a ^
    -DOPENSSL_SSL_LIBRARY:PATH=%OPENSSL%\libssl.dll.a ^
    -DOPENSSL_ROOT_DIR:PATH=%OPENSSL% ^
    -DOPENSSL_FOUND:BOOL=TRUE ^
    %LIBSSH2%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not "%1" == "" goto next

rem ======
rem  CURL
rem ======
:curl

if not exist %BUILD%\curl mkdir %BUILD%\curl
if errorlevel 1 goto error
cd %BUILD%\curl
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DBUILD_SHARED_LIBS:BOOL=YES ^
    -DENABLE_INET_PTON=OFF ^
    -DCMAKE_USE_OPENSSL=YES ^
    -DCURL_TARGET_WINDOWS_VERSION=0x400 ^
    -DCURL_LTO=NO ^
    -DCURL_ENABLE_SSL=YES ^
    -DZLIB_INCLUDE_DIR:PATH=%ZLIB%;%BUILD%\zlib ^
    -DZLIB_LIBRARY:PATH=%BUILD%\zlib\libzlib.dll.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    -DOPENSSL_INCLUDE_DIR:PATH=%OPENSSL%\include ^
    -DOPENSSL_CRYPTO_LIBRARY:PATH=%OPENSSL%\libcrypto.dll.a ^
    -DOPENSSL_SSL_LIBRARY:PATH=%OPENSSL%\libssl.dll.a ^
    -DOPENSSL_ROOT_DIR:PATH=%OPENSSL% ^
    -DOPENSSL_FOUND:BOOL=TRUE ^
    -DLIBSSH2_INCLUDE_DIR:PATH=%LIBSSH2%\include ^
    -DLIBSSH2_LIBRARY:PATH=%BUILD%\libssh2\src\liblibssh2.dll.a ^
    %CURL%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not exist %OUT%\include\curl mkdir %OUT%\include\curl
if errorlevel 1 goto error

if not "%1" == "" goto next

rem ==========================================================================
:installer

cd %~dp0_setup
if errorlevel 1 goto error

if exist %~dp0..\..\CD\SOFTWARE\GNU-NT4.EXE del %~dp0..\..\CD\SOFTWARE\GNU-NT4.EXE
if errorlevel 1 goto error

makensis setup.nsi
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
