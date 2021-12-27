@echo off
setlocal

set BASEDIR=%~dp0

set ZLIB=%BASEDIR%zlib-1.2.11
set BZIP2=%BASEDIR%bzip2-1.0.8
set OPENSSL=%BASEDIR%openssl-1.1.1m
set LIBSSH2=%BASEDIR%libssh2-1.10.0
set ICONV=libiconv-1.16
set CURL=%BASEDIR%curl-7.80.0
set EXPAT=%BASEDIR%expat-2.4.2
set NCURSES=%BASEDIR%ncurses-6.3
set PCRE2=%BASEDIR%pcre2-10.39
set LESS=%BASEDIR%less-590
set GIT=%BASEDIR%git-2.34.1

rem ==========================================================================

set TOOLCHAIN=%BASEDIR%..\Toolchain
set BUILD=%BASEDIR%_build

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
copy /b %BASEDIR%..\Toolchain\TDM-GCC\bin\libgcc_s_dw2-1.dll %BUILD%
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

rem =======
rem  BZIP2
rem =======
:bzip2

cd %BZIP2%

mingw32-make -j 4 -f Makefile
if errorlevel 1 goto error
mingw32-make -j 4 -f Makefile-libbz2_so
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

rem =======
rem  ICONV
rem =======
:iconv

cd %ICONV%
if errorlevel 1 goto error

sh configure ^
    --build=i586-mingw32msvc ^
    --host=i586-mingw32msvc ^
    --target=i586-mingw32msvc ^
    --enable-static=yes ^
    --enable-shared=yes ^
    --disable-fast-install ^
    --disable-nls ^
    MAKE=mingw32-make ^
    CFLAGS="-march=i586 -fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables" ^
    LDFLAGS="-Wl,--build-id=none -Wl,-s" ^
if errorlevel 1 goto error
mingw32-make
if errorlevel 1 goto error
cd src
if errorlevel 1 goto error
mingw32-make install
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

if not "%1" == "" goto next

rem =======
rem  EXPAT
rem =======
:expat

if not exist %BUILD%\expat mkdir %BUILD%\expat
if errorlevel 1 goto error
cd %BUILD%\expat
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DEXPAT_BUILD_TOOLS=FALSE ^
    -DEXPAT_BUILD_EXAMPLES=FALSE ^
    -DEXPAT_BUILD_TESTS=FALSE ^
    -DEXPAT_BUILD_PKGCONFIG=FALSE ^
    %EXPAT%
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not "%1" == "" goto next

rem =======
rem  PCRE2
rem =======
:pcre2

if not exist %BUILD%\pcre2 mkdir %BUILD%\pcre2
if errorlevel 1 goto error
cd %BUILD%\pcre2
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DBUILD_SHARED_LIBS=ON ^
    -DBUILD_STATIC_LIBS=ON ^
    -DPCRE2_NEWLINE=ANYCRLF ^
    -DPCRE2_BUILD_PCRE2GREP=OFF ^
    -DPCRE2_BUILD_TESTS=OFF ^
    -DZLIB_INCLUDE_DIR:PATH=%ZLIB%;%BUILD%\zlib ^
    -DZLIB_LIBRARY:PATH=%BUILD%\zlib\libzlib.dll.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    %PCRE2%
if errorlevel 1 goto error
mingw32-make
if errorlevel 1 goto error

if not "%1" == "" goto next

rem =========
rem  NCURSES
rem =========
:ncurses

cd %NCURSES%
if errorlevel 1 goto error

rmdir %NCURSES%\misc\_terminfo_

sh configure ^
    MAKE=mingw32-make ^
    CFLAGS="-fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -march=i586 -I%PCRE2%/src" ^
    LDFLAGS="-Wl,--build-id=none -Wl,-s -Wl,-L%BUILD:\=/%/pcre2" ^
    --without-ada ^
    --without-manpages ^
    --without-debug ^
    --without-cxx-binding ^
    --without-tests ^
    --with-pcre2 ^
    --with-shared ^
    --with-trace ^
    --with-panel-libname=pane ^
    --disable-exp-win32 ^
    --disable-home-terminfo ^
    --disable-getcap ^
    --disable-termcap ^
    --disable-hard-tabs ^
    --disable-leaks ^
    --enable-opaque-curses ^
    --enable-opaque-form ^
    --enable-opaque-menu ^
    --enable-opaque-panel ^
    --enable-database ^
    --enable-sp-funcs ^
    --enable-term-driver ^
    --enable-fvisibility ^
    --enable-interop
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error
mingw32-make install.data
if errorlevel 1 goto error

if not "%1" == "" goto next

rem ======
rem  LESS
rem ======
:less

cd %LESS%
if errorlevel 1 goto error

sh configure ^
    CFLAGS="-march=i586 -fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -I%NCURSES%/include -I%BUILD%/pcre2" ^
    LDFLAGS="-Wl,--build-id=none -Wl,-s -Wl,-L%NCURSES%/lib -Wl,-L%BUILD%/pcre2" ^
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not "%1" == "" goto next

rem =====
rem  GIT
rem =====
:git

if not exist %BUILD%\git mkdir %BUILD%\git
if errorlevel 1 goto error
cd %BUILD%\git
if errorlevel 1 goto error

cmake -G "MinGW Makefiles" ^
    -Wno-dev ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCMAKE_MAKE_PROGRAM=mingw32-make ^
    -DUSE_VCPKG=FALSE ^
    -DZLIB_INCLUDE_DIR:PATH=%ZLIB%;%BUILD%\zlib ^
    -DZLIB_LIBRARY:PATH=%BUILD%\zlib\libzlib.dll.a ^
    -DZLIB_FOUND:BOOL=TRUE ^
    -DCURL_INCLUDE_DIR:PATH=%CURL%\include ^
    -DCURL_LIBRARY:PATH=%BUILD%\curl\lib\libcurl.dll.a ^
    -DCURL_FOUND:BOOL=TRUE ^
    -DEXPAT_INCLUDE_DIR:PATH=%EXPAT%\lib ^
    -DEXPAT_LIBRARY:PATH=%BUILD%\expat\libexpat.dll.a ^
    -DEXPAT_FOUND:BOOL=TRUE ^
    -DNO_GETTEXT=YES ^
    -DBUILD_TESTING=NO ^
    %GIT%\contrib\buildsystems
if errorlevel 1 goto error
mingw32-make -j 4
if errorlevel 1 goto error

if not "%1" == "" goto next

rem ==========================================================================
:installer

cd %BASEDIR%_setup
if errorlevel 1 goto error

if exist %BASEDIR%..\..\CD\SOFTWARE\GNU-NT4.EXE del %BASEDIR%..\..\CD\SOFTWARE\GNU-NT4.EXE
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
