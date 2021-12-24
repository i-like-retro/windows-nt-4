@echo off
setlocal

set ZLIB=%~dp0zlib-1.2.11
set OPENSSL=%~dp0openssl-1.1.1m
set LIBSSH2=%~dp0libssh2-1.10.0
set CURL=%~dp0curl-7.80.0

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

cmake -G "MinGW Makefiles" ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DCMAKE_BUILD_TYPE=Release ^
    %ZLIB%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

cmake -E copy_if_different zlib.dll %OUT%\bin\zlib.dll
if errorlevel 1 goto error
cmake -E copy_if_different libzlib.dll.a %OUT%\lib\libz.a
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
    "--openssldir=C:\Program Files\GNU" ^
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

cmake -E copy_if_different libcrypto.a %OUT%\lib\libcryptostatic.a
if errorlevel 1 goto error
cmake -E copy_if_different libcrypto.dll.a %OUT%\lib\libcrypto.a
if errorlevel 1 goto error
cmake -E copy_if_different libeay11.dll %OUT%\bin
if errorlevel 1 goto error
cmake -E copy_if_different libssl.a %OUT%\lib\libsslstatic.a
if errorlevel 1 goto error
cmake -E copy_if_different libssl.dll.a %OUT%\lib\libssl.a
if errorlevel 1 goto error
cmake -E copy_if_different ssleay11.dll %OUT%\bin
if errorlevel 1 goto error
for %%f in (include\openssl\*.h) do cmake -E copy_if_different %%f %OUT%\include\openssl
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
    -DZLIB_INCLUDE_DIR:PATH=%OUT%\include ^
    -DZLIB_LIBRARY:PATH=%OUT%\lib\libz.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    -DOPENSSL_INCLUDE_DIR:PATH=%OUT%\include ^
    -DOPENSSL_CRYPTO_LIBRARY:PATH=%OUT%\libcrypto.a ^
    -DOPENSSL_SSL_LIBRARY:PATH=%OUT%\lib\libssl.a ^
    -DOPENSSL_ROOT_DIR:PATH=%OUT% ^
    -DOPENSSL_FOUND:BOOL=TRUE ^
    %LIBSSH2%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

cmake -E copy_if_different src\libssh2.dll %OUT%\bin
if errorlevel 1 goto error
cmake -E copy_if_different src\liblibssh2.dll.a %OUT%\lib\libssh2.a
if errorlevel 1 goto error
for %%f in (%LIBSSH2%\include\*.h) do cmake -E copy_if_different %%f %OUT%\include
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
    -DZLIB_INCLUDE_DIR:PATH=%OUT%\include ^
    -DZLIB_LIBRARY:PATH=%OUT%\lib\libz.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    -DOPENSSL_INCLUDE_DIR:PATH=%OUT%\include ^
    -DOPENSSL_CRYPTO_LIBRARY:PATH=%OUT%\libcrypto.a ^
    -DOPENSSL_SSL_LIBRARY:PATH=%OUT%\lib\libssl.a ^
    -DOPENSSL_ROOT_DIR:PATH=%OUT% ^
    -DOPENSSL_FOUND:BOOL=TRUE ^
    -DLIBSSH2_INCLUDE_DIR:PATH=%OUT%\include ^
    -DLIBSSH2_LIBRARY:PATH=%OUT%\lib\libssh2.a ^
    %CURL%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not exist %OUT%\include\curl mkdir %OUT%\include\curl
if errorlevel 1 goto error

cmake -E copy_if_different src\curl.exe %OUT%\bin
if errorlevel 1 goto error
cmake -E copy_if_different lib\libcurl.dll %OUT%\bin
if errorlevel 1 goto error
cmake -E copy_if_different lib\libcurl.dll.a %OUT%\lib\libcurl.a
if errorlevel 1 goto error
for %%f in (%CURL%\include\curl\*.h) do cmake -E copy_if_different %%f %OUT%\include\curl
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
