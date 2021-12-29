#ifndef NTCOMPAT_NTCOMPAT_H
#define NTCOMPAT_NTCOMPAT_H

// possible defines:
//  * NTCOMPAT_NEED_STRUCT_ADDINFO: include struct addrinfo
//  * NTCOMPAT_NEED_EXTENDED_NAME_FORMAT: include EXTENDED_NAME_FORMAT

#include <ntcompat/windows.h>
#include <ntcompat/crt.h>
#include <ntcompat/errno.h>

#ifndef NTCOMPAT_EXPORTS
#define NTCOMPAT_EXPORTS __declspec(dllimport)
#endif

NTCOMPAT_EXPORTS HANDLE NT4_GetCurrentThreadHandle(void);

#endif
