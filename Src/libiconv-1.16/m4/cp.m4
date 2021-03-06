dnl Copyright (C) 1993-2002 Free Software Foundation, Inc.
dnl This file is free software, distributed under the terms of the GNU
dnl General Public License.  As a special exception to the GNU General
dnl Public License, this file may be distributed as part of a program
dnl that contains a configuration script generated by Autoconf, under
dnl the same distribution terms as the rest of that program.

dnl From Bruno Haible, Marcus Daniels.

AC_PREREQ(2.13)

AC_DEFUN([CL_PROG_CP],
[AC_CACHE_CHECK(how to copy files, cl_cv_prog_cp, [
echo "blabla" > conftest.x
err=`sh -c "cp -p conftest.x conftest.y 2>&1"`
if test -z "$err"; then
  cl_cv_prog_cp='cp -p'
else
  cl_cv_prog_cp='cp'
fi
rm -f conftest*
])
CP="$cl_cv_prog_cp"
AC_SUBST(CP)dnl
])
