# Makefile to build cdrecord, mkisofs, etc. for Windows using Cygwin
# Copyright (c) 2004-2006 Ross Smith II (http://smithii.com). All rights reserved.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License 
# as published by the Free Software Foundation.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# $Id$

# see http://www.fokus.gmd.de/research/cc/glone/employees/joerg.schilling/private/cdrecord.html
# and http://people.mandrakesoft.com/~warly/files/cdrtools/archives/

APPLY_FORCE_UPPERCASE_PATCH	?=N
APPLY_DVDPATCH			?=N
APPLY_O_EXCL_PATCH		?=N
APPLY_WRITEMODE_PATCH		?=N

# ftp://ftp.berlios.de/pub/cdrecord/alpha/cdrtools-2.01.01a03.tar.bz2
CDRTOOLS_VER		?=2.01.01
CDRTOOLS_REL		?=a07
CDRTOOLS_DIR		?=cdrtools-${CDRTOOLS_VER}
CDRTOOLS_BZ2		?=${CDRTOOLS_DIR}${CDRTOOLS_REL}.tar.bz2
CDRTOOLS_URL		?=ftp://ftp.berlios.de/pub/cdrecord/alpha/${CDRTOOLS_BZ2}

CDRTOOLS_VER		=2.01
CDRTOOLS_REL		=
CDRTOOLS_URL		=ftp://ftp.berlios.de/pub/cdrecord/${CDRTOOLS_BZ2}

CYGWIN_ROOT		?=$(shell cygpath -m /)

#PKZIPC         	 ?=/cygdrive/c/PROGRA~1/pkware/pkzipc/pkzipc.exe
#PKZIPC_OPTS    	 ?=-add -path=none

DEST_ZIP		?=cdrtools-latest${PATCH_DESC}.zip

CDRTOOLS_GETLINE_PATCH	?=cdrtools-2.01.cygwin.patch
CDRTOOLS_GETLINE_URL	?=http://smithii.com/files/${CDRTOOLS_GETLINE_PATCH}

FORCE_UPPERCASE_PATCH	?=cdrtools-2.01-force_uppercase.patch
FORCE_UPPERCASE_URL	?=http://smithii.com/files/${FORCE_UPPERCASE_PATCH}

# http://people.mandriva.com/~warly/files/cdrtools/archives/cdrtools-2.01.01a04-dvd.patch.bz2

PATCH_REL			?=a04
DVDPATCH    		?=${CDRTOOLS_DIR}${PATCH_REL}-dvd.patch
DVDPATCH_BZ2		?=${DVDPATCH}.bz2
DVDPATCH_URL		?=http://people.mandriva.com/~warly/files/cdrtools/archives/${DVDPATCH_BZ2}

#DVDPATCH_PATCH		?=${DVDPATCH}.${CDRTOOLS_REL}.patch
#DVDPATCH_PATCH_URL	?=http://smithii.com/files/${DVDPATCH_PATCH}

O_EXCL_PATCH_REL	?=cdrtools-2.01a27

O_EXCL_PATCH    	?=${O_EXCL_PATCH_REL}-o_excl.patch
O_EXCL_PATCH_BZ2	?=${O_EXCL_PATCH}.bz2
O_EXCL_PATCH_URL	?=http://people.mandriva.com/~warly/files/cdrtools/archives/${O_EXCL_PATCH_BZ2}

#O_EXCL_PATCH_PATCH	?=${O_EXCL_PATCH}.${CDRTOOLS_REL}.patch
O_EXCL_PATCH_PATCH	?=${O_EXCL_PATCH}..patch
O_EXCL_PATCH_PATCH_URL	?=http://smithii.com/files/${O_EXCL_PATCH_PATCH}

WRITEMODE_PATCH_REL	?=cdrtools-2.01a27

WRITEMODE_PATCH    	?=${WRITEMODE_PATCH_REL}-writemode.patch
WRITEMODE_PATCH_BZ2	?=${WRITEMODE_PATCH}.bz2
WRITEMODE_PATCH_URL	?=http://people.mandriva.com/~warly/files/cdrtools/archives/${WRITEMODE_PATCH_BZ2}

BUNZIP2			?=bunzip2 -f -k
PATCH			?=patch
PATCH_OPTS		?=--force 
RAR			?=rar 
TAR			?=tar
TAR_OPTS		?=--overwrite -x -j -v -f
TOUCH			?=touch
WGET			?=wget -N

UPX			?=upx
UPX_OPTS		?=--best
ZIP                     ?=zip
ZIP_OPTS                ?=-9jquX

#include local.mak

CDRTOOLS_DIR_TOUCH	=${CDRTOOLS_DIR}.touch
CDRTOOLS_GETLINE_TOUCH=${CDRTOOLS_GETLINE_PATCH}.touch
DVDPATCH_TOUCH		=${DVDPATCH}.touch
#DVDPATCH_PATCH_TOUCH	=${DVDPATCH_PATCH}.touch
FORCE_UPPERCASE_TOUCH	=${FORCE_UPPERCASE_PATCH}.touch
EXES_TOUCH		=exes.touch
O_EXCL_PATCH_TOUCH	=${O_EXCL_PATCH}.touch
UPX_TOUCH		=upx.touch
WRITEMODE_PATCH_TOUCH	=${WRITEMODE_PATCH}.touch

DIR=OBJ/i686-cygwin32_nt-gcc

EXES=\
	${CDRTOOLS_DIR}/cdda2wav/${DIR}/cdda2wav.exe \
	${CDRTOOLS_DIR}/cdrecord/${DIR}/cdrecord.exe \
	${CDRTOOLS_DIR}/inc/${DIR}/align_test.exe \
	${CDRTOOLS_DIR}/inc/${DIR}/avoffset.exe \
	${CDRTOOLS_DIR}/mkisofs/diag/${DIR}/devdump.exe \
	${CDRTOOLS_DIR}/mkisofs/diag/${DIR}/isodebug.exe \
	${CDRTOOLS_DIR}/mkisofs/diag/${DIR}/isodump.exe \
	${CDRTOOLS_DIR}/mkisofs/diag/${DIR}/isoinfo.exe \
	${CDRTOOLS_DIR}/mkisofs/diag/${DIR}/isovfy.exe \
	${CDRTOOLS_DIR}/mkisofs/${DIR}/mkisofs.exe \
	${CDRTOOLS_DIR}/readcd/${DIR}/readcd.exe \
	${CDRTOOLS_DIR}/rscsi/${DIR}/rscsi.exe \
	${CDRTOOLS_DIR}/scgcheck/${DIR}/scgcheck.exe \
	${CDRTOOLS_DIR}/scgskeleton/${DIR}/skel.exe

PREREQUISITES=${CDRTOOLS_DIR_TOUCH} ${CDRTOOLS_GETLINE_TOUCH}

ifeq ('$(APPLY_FORCE_UPPERCASE_PATCH)', 'Y')
	SOURCE_MODIFIED=Y
	PREREQUISITES+=${FORCE_UPPERCASE_TOUCH}
endif

ifeq ('$(APPLY_DVDPATCH)', 'Y')
	SOURCE_MODIFIED=Y
	PREREQUISITES+=${DVDPATCH_TOUCH}
#	PREREQUISITES+=${DVDPATCH_PATCH_TOUCH}
endif

ifeq ('$(APPLY_O_EXCL_PATCH)', 'Y')
	SOURCE_MODIFIED=Y
	PREREQUISITES+=${O_EXCL_PATCH_TOUCH}
	PREREQUISITES+=${O_EXCL_PATCH_PATCH_TOUCH}
endif

ifeq ('$(APPLY_WRITEMODE_PATCH)', 'Y')
	SOURCE_MODIFIED=Y
	PREREQUISITES+=${WRITEMODE_PATCH_TOUCH}
endif

ifeq ('$(SOURCE_MODIFIED)', 'Y')
MAKE_OPTS="SOURCE_MODIFIED=1"
endif

.PHONY:
	all \
	clean \
	dist \
	exes_dir \
	upx \
	rar \
	realclean \
	tidy \
	zip

all:	exes_dir

exes_dir:	${EXES_TOUCH}

${CDRTOOLS_BZ2}:
	${WGET} ${CDRTOOLS_URL}

${CDRTOOLS_DIR_TOUCH}:	${CDRTOOLS_BZ2}
	echo DEST_ZIP=${DEST_ZIP}
	${TAR} ${TAR_OPTS} $<
	chmod -R u+w ${CDRTOOLS_DIR}
	rm -f cdrtools
	ln -s ${CDRTOOLS_DIR} cdrtools
	${TOUCH} $@

${CDRTOOLS_GETLINE_PATCH}:
	${WGET} ${CDRTOOLS_GETLINE_URL}

${CDRTOOLS_GETLINE_TOUCH}:		${CDRTOOLS_GETLINE_PATCH} ${CDRTOOLS_DIR_TOUCH}
	TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR} -p1 --input=../$<
	${TOUCH} $@

${FORCE_UPPERCASE_PATCH}:
	${WGET} ${FORCE_UPPERCASEPATCH_URL}

${FORCE_UPPERCASE_TOUCH}:		${FORCE_UPPERCASE_PATCH} ${CDRTOOLS_DIR_TOUCH}
	TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR} -p1 --input=../$<
	${TOUCH} $@

${DVDPATCH_BZ2}:
	${WGET} ${DVDPATCH_URL}

${DVDPATCH}:		${DVDPATCH_BZ2} ${CDRTOOLS_DIR_TOUCH}
	${BUNZIP2} $<
	${TOUCH} $@ 
	
${DVDPATCH_TOUCH}:		${DVDPATCH} ${CDRTOOLS_DIR_TOUCH}
	TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR} -p1 --input=../$<
	${TOUCH} $@

#${DVDPATCH_PATCH}:
#	${WGET} ${DVDPATCH_PATCH_URL}

#${DVDPATCH_PATCH_TOUCH}:		${DVDPATCH_TOUCH} ${DVDPATCH_PATCH} ${CDRTOOLS_DIR_TOUCH} 
#	cd ${CDRTOOLS_DIR}/cdrecord && \
#		${PATCH} -p0 <../../${DVDPATCH_PATCH}
#	${TOUCH} $@

${O_EXCL_PATCH_BZ2}:
	${WGET} ${O_EXCL_PATCH_URL}

${O_EXCL_PATCH}:		${O_EXCL_PATCH_BZ2} ${CDRTOOLS_DIR_TOUCH}
	${BUNZIP2} $<
	${TOUCH} $@ 

${O_EXCL_PATCH_TOUCH}:		${O_EXCL_PATCH} ${CDRTOOLS_DIR_TOUCH}
	-TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR} -p1 --input=../$<
	${TOUCH} $@

${O_EXCL_PATCH_PATCH}:
	${WGET} ${O_EXCL_PATCH_PATCH_URL}

${O_EXCL_PATCH_PATCH_TOUCH}:		${O_EXCL_PATCH_PATCH} ${O_EXCL_PATCH_TOUCH} ${CDRTOOLS_DIR_TOUCH} 
	TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR}/libscg -p0 --input=../../$<
	#cd ${CDRTOOLS_DIR}/libscg && \
	#	${PATCH} -p0 <../../${O_EXCL_PATCH_PATCH}
	${TOUCH} $@

${WRITEMODE_PATCH_BZ2}:
	${WGET} ${WRITEMODE_PATCH_URL}

${WRITEMODE_PATCH}:		${WRITEMODE_PATCH_BZ2} ${CDRTOOLS_DIR_TOUCH}
	${BUNZIP2} $<
	${TOUCH} $@ 

${WRITEMODE_PATCH_TOUCH}:		${WRITEMODE_PATCH} ${CDRTOOLS_DIR_TOUCH}
	-TMPDIR=/tmp ${PATCH} ${PATCH_OPTS} --directory=${CDRTOOLS_DIR} -p1 --input=../$<
	${TOUCH} $@

${EXES}:	${PREREQUISITES}
	cd ${CDRTOOLS_DIR} && \
		make ${MAKE_OPTS}

YYMMDD=$(shell date +"%y%m%d")
HHMM=$(shell echo ${CDRTOOLS_VER} | perl -n -e '/([0-9]+)\.([0-9]+)/ ; printf "%02d%02d", $$1, $$2;')
  
${EXES_TOUCH}:	${EXES}
	-mkdir -p exes
	find ${CDRTOOLS_DIR} -name '*.exe' -type f | xargs cp --target-directory=exes
	#${TOUCH} -r ${CDRTOOLS_BZ2} exes/*.exe
	-${TOUCH} -t ${YYMMDD}${HHMM} exes/*.exe
	ls -l exes
	${TOUCH} $@

${UPX_TOUCH}:	${EXES_TOUCH}
	-cd exes && \
		${UPX} ${UPX_OPTS} *.exe
	#${TOUCH} -r ${CDRTOOLS_BZ2} exes/*.exe
	-${TOUCH} -t ${YYMMDD}${HHMM} exes/*.exe
	${TOUCH} $@

upx:	${UPX_TOUCH}

X_FILES=${CDRTOOLS_DIR}/COPYING \
		${CDRTOOLS_DIR}/README.win32 \
		${CDRTOOLS_DIR}/AN-${CDRTOOLS_VER}${CDRTOOLS_REL} \
		${CDRTOOLS_GETLINE_PATCH} \
		/etc/default/cdrecord \
		cdrecord.mak

ZIP_FILES=${X_FILES} /bin/cygwin1.dll

${DEST_ZIP}:	upx ${EXES_TOUCH} ${ZIP_FILES}
	-${ZIP} ${ZIP_OPTS} ${DEST_ZIP} exes/*.exe /bin/cygwin1.dll macfiles.txt
	-${ZIP} -l ${ZIP_OPTS} ${DEST_ZIP} ${X_FILES}

	#	${DVDPATCH_PATCH}

zip:	${DEST_ZIP}

dist:	zip

tidy:
	rm -f \
	${CDRTOOLS_DIR_TOUCH} \
	${DVDPATCH_TOUCH} \
	${EXES_TOUCH} \
	${FORCE_UPPERCASE_TOUCH} \
	${O_EXCL_PATCH_TOUCH} \
	${UPX_TOUCH} \
	${WRITEMODE_PATCH_TOUCH}

clean:	tidy
	rm -fr ${CDRTOOLS_DIR} exes

realclean:	clean
	rm -fr \
		exes \
		${CDRTOOLS_BZ2}
