
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

System updates
==============

The first thing you would like to do after you install fresh Windows NT 4.0
is to install latest system and security updates.

The first thing to consider is of course the latest Service Pack 6A for
the system: [SP6AI386.EXE](CD/SOFTWARE/UPDATES/SP6AI386.EXE). Don't forget to
restart after installing the Service Pack. **On Windows NT 4 it is very important to
restart each time an update is installed (there are exceptions though, see below),
because otherwise you may end up with a newer DLL being overriden by an older update.**

After you install latest Service Pack, good idea will be to install Post-SP6A
Service Rollup release. This is the collection of patches, prepared by Microsoft
after SP6 came out. This was intended to be SP7, but it never seen the light of the day.
For more information you can check out the corresponding
[KB article](https://github.com/jeffpar/kbarchive/tree/master/kb/299/Q299444).
You can find it here: [Q299444I.EXE](CD/SOFTWARE/UPDATES/Q299444I.EXE). Don't forget
to restart after installation as well.
