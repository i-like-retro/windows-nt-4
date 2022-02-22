#ifndef PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
#define PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
#pragma once

/*
platform.headers.hpp

Platform headers
*/
/*
Copyright © 2020 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "disable_warnings_in_std_begin.hpp"
//----------------------------------------------------------------------------

#ifdef __GNUC__
#include <w32api.h>
#define _W32API_VER (100*(__W32API_MAJOR_VERSION) + (__W32API_MINOR_VERSION))
#if _W32API_VER < 314
#error w32api-3.14 (or higher) required
#endif
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE
#define WINVER       0x0603
#define _WIN32_WINNT 0x0603
#define _WIN32_IE    0x0700
#endif // __GNUC__

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WIN32_NO_STATUS //exclude ntstatus.h macros from winnt.h
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winioctl.h>
#include <mmsystem.h>
#include <wininet.h>
#include <winspool.h>
#include <setupapi.h>
#include <aclapi.h>
#include <sddl.h>
#include <dbt.h>
#include <lm.h>
#define SECURITY_WIN32
#include <security.h>
#define PSAPI_VERSION 1
#include <psapi.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <userenv.h>
#include "dbghelp.h"
/*#include <dwmapi.h>*/
/*#include <restartmanager.h>*/
#include <commdlg.h>
/*#include <winternl.h>*/
/*#include <versionhelpers.h>*/
#include <virtdisk.h>
/*#include <ntstatus.h>*/
/*#include <cfgmgr32.h>*/
#include <ddk/ntddmmc.h>
#include <ddk/ntddscsi.h>
#include <ddk/ntapi.h>
#include <ddk/ntifs.h>
/*#include <lmdfs.h>*/

#define _NTSCSI_USER_MODE_

#ifdef _MSC_VER
#include <scsi.h>
#endif // _MSC_VER

#ifdef __GNUC__
#include <ntdef.h>
// Workaround for MinGW, see a66e40
// Their loony headers are unversioned,
// so the only way to make it compatible
// with both old and new is this madness:
/*#include <netfw.h>*/
#ifndef __INetFwProduct_FWD_DEFINED__
#define _LBA
#define _MSF
#endif
#include <ddk/scsi.h>
#include <ddk/ntddstor.h>
#ifndef __INetFwProduct_FWD_DEFINED__
#undef _MSF
#undef _LBA
#endif
#endif // __GNUC__

#include "platform.sdk.hpp"

//----------------------------------------------------------------------------
#include "disable_warnings_in_std_end.hpp"

#undef far
#undef near

typedef enum _STREAM_INFO_LEVELS {
  FindStreamInfoStandard,
  FindStreamInfoMaxInfoLevel
} STREAM_INFO_LEVELS;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION {
  ULONG         NextEntryOffset;
  ULONG         FileIndex;
  LARGE_INTEGER CreationTime;
  LARGE_INTEGER LastAccessTime;
  LARGE_INTEGER LastWriteTime;
  LARGE_INTEGER ChangeTime;
  LARGE_INTEGER EndOfFile;
  LARGE_INTEGER AllocationSize;
  ULONG         FileAttributes;
  ULONG         FileNameLength;
  ULONG         EaSize;
  CCHAR         ShortNameLength;
  WCHAR         ShortName[12];
  LARGE_INTEGER FileId;
  WCHAR         FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef enum _HEAP_INFORMATION_CLASS {
  HeapCompatibilityInformation = 0,
  HeapEnableTerminationOnCorruption = 1,
  HeapOptimizeResources = 3,
  HeapTag
} HEAP_INFORMATION_CLASS;

#define RM_SESSION_KEY_LEN	sizeof(GUID)
#define CCH_RM_SESSION_KEY	(RM_SESSION_KEY_LEN*2)
#define CCH_RM_MAX_APP_NAME 	255
#define CCH_RM_MAX_SVC_NAME 	63

typedef enum _RM_APP_TYPE {
    RmUnknownApp = 0,
    RmMainWindow = 1,
    RmOtherWindow = 2,
    RmService = 3,
    RmExplorer = 4,
    RmConsole = 5,
    RmCritical = 1000
} RM_APP_TYPE;

typedef struct _RM_UNIQUE_PROCESS {
    DWORD dwProcessId;
    FILETIME ProcessStartTime;
} RM_UNIQUE_PROCESS, *PRM_UNIQUE_PROCESS;

typedef struct _RM_PROCESS_INFO{
    RM_UNIQUE_PROCESS Process;
    WCHAR strAppName[CCH_RM_MAX_APP_NAME+1];
    WCHAR strServiceShortName[CCH_RM_MAX_SVC_NAME+1];
    RM_APP_TYPE ApplicationType;
    ULONG AppStatus;
    DWORD TSSessionId;
    BOOL bRestartable;
} RM_PROCESS_INFO, *PRM_PROCESS_INFO;

typedef struct _WIN32_FIND_STREAM_DATA
{
    LARGE_INTEGER StreamSize;
    WCHAR cStreamName[MAX_PATH + 36];
}
WIN32_FIND_STREAM_DATA, *PWIN32_FIND_STREAM_DATA;

typedef struct _DFS_STORAGE_INFO {
  ULONG  State;
  LPWSTR ServerName;
  LPWSTR ShareName;
} DFS_STORAGE_INFO, *PDFS_STORAGE_INFO, *LPDFS_STORAGE_INFO;

#ifndef DFS_STORAGE_STATE_OFFLINE
#define DFS_STORAGE_STATE_OFFLINE 1
#endif

#ifndef DFS_STORAGE_STATE_ACTIVE
#define DFS_STORAGE_STATE_ACTIVE 4
#endif

typedef struct _DFS_INFO_3 {
  LPWSTR             EntryPath;
  LPWSTR             Comment;
  DWORD              State;
  DWORD              NumberOfStorages;
  LPDFS_STORAGE_INFO Storage;
} DFS_INFO_3, *PDFS_INFO_3, *LPDFS_INFO_3;

#define REPLACEFILE_IGNORE_ACL_ERRORS 0x00000004

#define VOLUME_NAME_GUID 0x1
#define GetFinalPathNameByHandle GetFinalPathNameByHandleW

#ifndef IO_REPARSE_TAG_HSM
#define IO_REPARSE_TAG_HSM 0xC0000004
#endif
#ifndef IO_REPARSE_TAG_HSM2
#define IO_REPARSE_TAG_HSM2 0x80000006
#endif
#ifndef IO_REPARSE_TAG_SIS
#define IO_REPARSE_TAG_SIS 0x80000007
#endif
#ifndef IO_REPARSE_TAG_WIM
#define IO_REPARSE_TAG_WIM 0x80000008
#endif
#ifndef IO_REPARSE_TAG_CSV
#define IO_REPARSE_TAG_CSV 0x80000009
#endif
#ifndef IO_REPARSE_TAG_DFS
#define IO_REPARSE_TAG_DFS 0x8000000A
#endif
#ifndef IO_REPARSE_TAG_DFSR
#define IO_REPARSE_TAG_DFSR 0x80000012
#endif
#ifndef IO_REPARSE_TAG_DEDUP
#define IO_REPARSE_TAG_DEDUP 0x80000013
#endif
#ifndef IO_REPARSE_TAG_NFS
#define IO_REPARSE_TAG_NFS 0x80000014
#endif

#ifndef SYMLINK_FLAG_RELATIVE
#define SYMLINK_FLAG_RELATIVE 1
#endif

#ifndef LOCALE_INVARIANT
#define LOCALE_INVARIANT 0x007F
#endif
#ifndef LINGUISTIC_IGNORECASE
#define LINGUISTIC_IGNORECASE 0x00000010
#endif
#ifndef NORM_LINGUISTIC_CASING
#define NORM_LINGUISTIC_CASING 0x08000000
#endif

#ifndef SORT_DIGITSASNUMBERS
#define SORT_DIGITSASNUMBERS 0x00000008
#endif

#ifndef EXCEPTION_READ_FAULT
#define EXCEPTION_READ_FAULT  0
#endif
#ifndef EXCEPTION_WRITE_FAULT
#define EXCEPTION_WRITE_FAULT   1
#endif
#ifndef EXCEPTION_EXECUTE_FAULT
#define EXCEPTION_EXECUTE_FAULT   8
#endif
#ifndef EXCEPTION_POSSIBLE_DEADLOCK
#define EXCEPTION_POSSIBLE_DEADLOCK 0xC0000194
#endif

typedef enum _RM_REBOOT_REASON {
  RmRebootReasonNone = 0x0,
  RmRebootReasonPermissionDenied = 0x1,
  RmRebootReasonSessionMismatch = 0x2,
  RmRebootReasonCriticalProcess = 0x4,
  RmRebootReasonCriticalService = 0x8,
  RmRebootReasonDetectedSelf
} RM_REBOOT_REASON;

#ifndef COPY_FILE_ALLOW_DECRYPTED_DESTINATION
#define COPY_FILE_ALLOW_DECRYPTED_DESTINATION 8
#endif

#ifndef INPUTLANGCHANGE_FORWARD
#define INPUTLANGCHANGE_FORWARD 2
#endif
#ifndef INPUTLANGCHANGE_BACKWARD
#define INPUTLANGCHANGE_BACKWARD 4
#endif


// FIXME
#define IsWindowsServer() (false)
#define IsWindowsXPOrGreater() (false)
#define IsWindowsVistaOrGreater() (false)
#define IsWindows7OrGreater() (false)
#define IsWindows8OrGreater() (false)
#define IsWindows10OrGreater() (false)

#endif // PLATFORM_HEADERS_HPP_28623022_12EB_4D53_A153_16CAC90C0710
