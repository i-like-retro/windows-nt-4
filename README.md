
What is this?
=============

This repository contains some hints, tips, links and software to
help you setup at least somehow comfortable environment on
Windows NT 4.0 in year 2022.

I am trying to collect in one place all the latest information on
required updates, problems, available software, etc. I also try to
compiled and backport some modern open source software to run on
Windows NT 4.0.

Why? Because I like this retro OS :)

CD Image
========

For your convenience, you can build an ISO image with all useful
stuff in this repository to burn on CD and apply to your Windows NT4
installation.

TBD: instructions

Important Updates
=================

The first thing you would like to do after you install fresh Windows NT 4.0
is to install latest system and security updates.

The first thing to consider is of course the latest Service Pack 6A for
the system: [SP6AI386.EXE](CD/SOFTWARE/UPDATES/SP6AI386.EXE). Don't forget to
restart after installing the Service Pack. **On Windows NT 4 it is very
important to restart each time an update is installed (there are exceptions
though, see below), because otherwise you may end up with a newer DLL being
overriden by an older update.**

After you install latest Service Pack, good idea will be to install Post-SP6A
Service Rollup release. This is the collection of patches, prepared by
Microsoft after SP6 came out. This was intended to be SP7, but it never seen
the light of the day. For more information you can check out the corresponding
[KB article](https://github.com/jeffpar/kbarchive/tree/master/kb/299/Q299444).
You can find it here: [Q299444I.EXE](CD/SOFTWARE/UPDATES/Q299444I.EXE).
Don't forget to restart after installation as well.

After these two important big patches are installed, its time for smaller
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
