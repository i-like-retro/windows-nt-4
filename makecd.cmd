@echo off

cd %~dp0CD
if errorlevel 1 exit /B 1

%~dp0Toolchain\CDRTOOLS\mkisofs ^
    -iso-level 1 ^
    -no-iso-translate ^
    -V "NT4SOFT" ^
    -volset "NT4SOFT" ^
    -publisher "I LIKE RETRO" ^
    -preparer "HTTPS://GITHUB.COM/I-LIKE-RETRO/WINDOWS-NT-4" ^
    -appid "MKISOFS V2.01 (I686-PC-CYGWIN)" ^
    -sysid "" ^
    -disable-deep-relocation ^
    -omit-version-number ^
    -o ../nt4soft.iso ^
    .
if errorlevel 1 exit /B 1
