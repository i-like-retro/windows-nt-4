@echo off

cd %~dp0
if errorlevel 1 exit /B 1

if not exist _diff mkdir _diff
if errorlevel 1 exit /B 1

git diff 26f5b80cafcc0a2f106051a9285ad84d45d02d68 -- zlib-1.2.11 > _diff/zlib-1.2.11.diff
if errorlevel 1 exit /B 1

git diff 81df82167b5ae554ac238ed31b909c6a211e15b9 -- openssl-1.1.1m > _diff/openssl-1.1.1m.diff
if errorlevel 1 exit /B 1

git diff dc9e6530dea8ee3e7822c3611b55f44fe405fc26 -- ncurses-6.3 > _diff/ncurses-6.3.diff
if errorlevel 1 exit /B 1

git diff 4473eaf416dadee11caf0d654394f25c8edc1234 -- libssh2-1.10.0 > _diff/libssh2-1.10.0.diff
if errorlevel 1 exit /B 1

git diff 2bbff5eaedfde1bdcbe39ebbec5cbb3c13f9909b -- less-590 > _diff/less-590.diff
if errorlevel 1 exit /B 1

git diff c979d25756c577aa470cd110bbe0538a7a7f6dfa -- git-2.34.1 > _diff/git-2.34.1.diff
if errorlevel 1 exit /B 1

git diff 3ed5a4df74ad572a7fd323cd3cffa236e7328ae1 -- expat-2.4.2 > _diff/expat-2.4.2.diff
if errorlevel 1 exit /B 1

git diff 3906b2888c223ae33dca7aeef4fa7643950675b1 -- curl-7.80.0 > _diff/curl-7.80.0.diff
if errorlevel 1 exit /B 1

git diff 04020899dbf294ef8ed7cbb313e4bc02851b8a02 -- cmake-3.5.2 > _diff/cmake-3.5.2.diff
if errorlevel 1 exit /B 1

git diff 3e782653bfe06b9e6bde583f15a69cc4cedce46b -- 7z2106 > _diff/7z2106.diff
if errorlevel 1 exit /B 1