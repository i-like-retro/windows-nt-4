Strawberry Perl 5.12.3.0 Portable README
===============================

What is Strawberry Perl?
------------------------

* 'Perl' is a programming language suitable for writing simple scripts as well
  as complex applications. See http://perldoc.perl.org/perlintro.html

* 'Strawberry Perl' is a perl enviroment for Microsoft Windows containing all
  you need to run and develop perl applications. It is designed to be as close
  as possible to perl enviroment on UNIX systems. See http://strawberryperl.com/

Installation instructions: (.zip distribution only)
---------------------------------------------------

* If installing this version from a .zip file, you MUST install it to
  C:\strawberry\ and then run some commands:

  cd C:\strawberry
  perl\bin\perl.exe update_env.pl.bat
   
  (You can specify " --nosystem" after update_env.pl.bat to install Strawberry 
  Perl's environment variables for the current user only.)

* If the fixed installation path C:\strawberry\ does not suit you, go
  to http://strawberryperl.com/releases.html and try "Strawberry Perl Portable 
  Edition"


How to use Strawberry Perl?
---------------------------

* In the command prompt window you can:

  1. run any perl script by launching
  
     c:\> perl c:\path\to\script.pl

  2. install additional perl modules (libraries) from http://www.cpan.org/ by

     c:\> cpan Module::Name
  
  3. run other tools included in Strawberry Perl like: perldoc, gcc, dmake ...

* You'll need a text editor to create perl scripts.  One is NOT included with 
Strawberry Perl.  A few options are Padre (which can be installed by running 
"cpan Padre" from the command prompt for 32-bit versions) and Notepad++ 
(which is downloadable at notepad-plus.sourceforge.net), which both include 
syntax highlighting for perl scripts.  You can even use Notepad, if you wish.
