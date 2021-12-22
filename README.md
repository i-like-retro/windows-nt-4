
What is this?
=============

This repository contains some hints, tips, links and software to
help you setup at least somehow comfortable environment in
Windows NT 4 Workstation in year 2022.

I am trying to collect in one place all the latest information on
required updates, problems, available software, etc. I also try to
compile and backport some modern open source software to run on
Windows NT 4.

Why? Because I like this retro OS :)

If you would like to have some other open source software backported and
compiled for Windows NT 4, create a GitHub issue. And I will take a look!

CD Image
========

For your convenience, you can build an ISO image with all useful
stuff in this repository to burn on CD and apply to your Windows NT4
installation at once.

TBD: instructions

Updates
=======

Important Updates
-----------------

The first thing you would like to do after you install fresh Windows NT 4
is to install latest system and security updates. These updates are highly
recommended to install. They improve user experience and increase
compatibility with software.

**I**) The first thing to consider is of course the latest Service Pack 6A for
the system: [SP6AI386.EXE](CD/SOFTWARE/UPDATES/SP6AI386.EXE). Don't forget to
restart after installing the Service Pack. **On Windows NT 4 it is very
important to restart each time an update is installed (there are exceptions
though, see below), because otherwise you may end up with a newer DLL being
overriden by an older update.**

**II**) After you install latest Service Pack, good idea will be to install Post-SP6A
Service Rollup release. This is the collection of patches, prepared by
Microsoft after SP6 came out. This was intended to be SP7, but it never seen
the light of the day. For more information you can check out the corresponding
[KB article](https://github.com/jeffpar/kbarchive/tree/master/kb/299/Q299444).
You can find it here: [Q299444I.EXE](CD/SOFTWARE/UPDATES/Q299444I.EXE).
Don't forget to restart after installation as well.

**III**) After these two important big patches are installed, its time for smaller
ones. After Service Rollup came out, there were some smaller security updates
that are nice to install as well. **NOTE**: Each of the updates requires
reboot, but there is a helper tool from Microsoft you could use to avoid
restarting after each update. Download the
[QCHAIN.EXE](CD/SOFTWARE/UPDATES/QCHAIN.EXE) file and run it after installing
all the updates but before restarting the system. You may need to give
proper command line parameters to the updates to prevent them from restarting
the system (usually its "-z" flag). The version of QCHAIN.EXE in this
repository is the newer one from the Q815062 update, with some bugfixes.
For more information search for "QCHAIN.EXE" in the following articles:
[1](https://support.microsoft.com/en-us/topic/how-to-install-multiple-windows-updates-or-hotfixes-with-only-one-reboot-6247def4-7f39-c1a0-efe5-61f82849fb7c),
[2](https://docs.microsoft.com/en-us/security-updates/securitybulletinsummaries/2005/ms05-nov).

* [Q810833.EXE](CD/SOFTWARE/UPDATES/Q810833.EXE) [Unchecked buffer in locator service could lead to code execution](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2003/ms03-001)
* [Q817606.EXE](CD/SOFTWARE/UPDATES/Q817606.EXE) [Buffer Overrun in Windows Could Lead to Data Corruption](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2003/ms03-024)
* [KB828035.EXE](CD/SOFTWARE/UPDATES/KB828035.EXE) [Buffer Overrun in Messenger Service Could Allow Code Execution](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2003/ms03-043)
* [KB828741.EXE](CD/SOFTWARE/UPDATES/KB828741.EXE) [Cumulative Update for Microsoft RPC/DCOM](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2004/ms04-012)
* [KB835732.EXE](CD/SOFTWARE/UPDATES/KB835732.EXE) [Security Update for Microsoft Windows](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2004/ms04-011)
* [KB839645.EXE](CD/SOFTWARE/UPDATES/KB839645.EXE) [Vulnerability in Windows Shell Could Allow Remote Code Execution](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2004/ms04-024)
* [KB841356.EXE](CD/SOFTWARE/UPDATES/KB841356.EXE) [Vulnerability in Windows Shell Could Allow Remote Code Execution](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2004/ms04-037)
* [KB841872.EXE](CD/SOFTWARE/UPDATES/KB841872.EXE) [Vulnerability in POSIX Could Allow Code Execution](https://docs.microsoft.com/en-us/security-updates/securitybulletins/2004/ms04-020)

**IV**) Now its time to install IE4. **Make sure to do this before you install IE6,
otherwise you won't be able to install it later!** This version will update your shell
with support for 256-colored icons, font smoothing and other improvements. Just make
sure to enable this feature during installation. To install, run IE4SETUP.EXE from
the [IE4](CD/SOFTWARE/UPDATES/IE4) directory.

**V**) After IE4 has been installed, you can install IE6 SP1 to get the latest
supported Internet Explorer on your system. This won't really help to browse
Internet, but it will install newer versions of some important system
components and improve overall compatibility with the software. Run IE6SETUP.EXE
from the [IE6SP1NT](CD/SOFTWARE/UPDATES/IE6SP1NT) directory.

Optional Updates
----------------

These updates are not necessary to install, but may be needed by some software
and are still recommended.

**I**) To support MSI installers, it is also highly recommended to install
MSI 2.0 update. It won't support newest MSI packages, but will definitely
improve a range of software that can be installed on the Machine. Grab it here:
[INSTMSIW.EXE](CD/SOFTWARE/UPDATES/INSTMSIW.EXE).

**II**) Microsoft Data Access Components
([MDAC_TYP.EXE](CD/SOFTWARE/UPDATES/MDAC_TYP.EXE)) and Microsoft Jet database
engine ([JETSETUP.EXE](CD/SOFTWARE/UPDATES/JETSETUP.EXE)) are
used by some software, most notably big Microsoft software packages like
Microsoft Office and Visual Studio.

**III**) .NET Framework. Yes, you can install it on Windows NT 4!
Unfortunately, the latest version you can install is 1.1. But still, it
improves compatibility considerably and is recomended for installation.
Installers for .NET Framework 1.0
([DOTNETFX.EXE](CD/SOFTWARE/UPDATES/DOTNETFX.EXE)) and .NET Framework 1.1
([DOTNET11.EXE](CD/SOFTWARE/UPDATES/DOTNET11.EXE)) are available.

**IV**) [MSIMG32.DLL](CD/SOFTWARE/UPDATES/DLLS/MSIMG32.DLL). Some software
requires this library. Multiple versions of the library can be found and most
of them require Windows 2000 or newer, and will not work on Windows NT 4. On
the other hand, version in this repository works perfectly. Just copy it to
your `\WINNT\SYSTEM32` directory.

Software
========

Compression
-----------

* [7zip 21.06](CD/SOFTWARE/COMPRESS/7Z2106.EXE) `2021-11-29`: free and open
source compression tool. Supports 7z, zip, rar and many other file formats.
7zip [source code](Src/7z2106) has been modified to build with Visual C++ 6.0
and work under Windows NT 4.

* [WinRAR 3.93](CD/SOFTWARE/COMPRESS/WRAR393.EXE) `[OLD]` `2009`: shareware
compression tool. This is the latest version that works under Windows NT 4.
Note: due to its age it doesn't support RAR4 archives.

Software Development
--------------------

* [CMake 3.52](CD/SOFTWARE/DEVEL/CMAKE352.EXE) `2018-12-03`: free and open
source tool for generating project files for C and C++ projects. This is not
the latest version, but its the latest one to support Visual C++ 6.0 and
Visual Studio 7 (aka .NET 2002). So there is little benefit in using newer
versions. CMake [source code](Src/cmake-3.5.2) have been modified to build and
work under Windows NT 4.

* [HxD 1.7.7.0](CD/SOFTWARE/DEVEL/HXD1770.EXE) `2009-04-03`: freeware hex
editor and disk editor (for more information see https://mh-nexus.de/en/hxd).
This is the latest version to work on Windows NT 4.

* [Nullsoft Scriptable Install System 2.51](CD/SOFTWARE/DEVEL/NSIS251.EXE)
`2016-04-20`: free and open source system for creating installers for Windows
applications. This is the latest version of NSIS 2.x. Actually NSIS 3.08
([NSIS308.EXE](CD/SOFTWARE/DEVEL/NSIS308.EXE) ` 2021-09-25`) also installs
and works perfectly on Windows NT 4, but it seems to be unable to parse
scripts used by CMake. For this reason it is recommended to install NSIS 2.x
if you plan to use it together with CMake.
