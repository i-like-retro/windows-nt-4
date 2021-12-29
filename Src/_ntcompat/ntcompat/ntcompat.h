#ifndef NTCOMPAT_NTCOMPAT_H
#define NTCOMPAT_NTCOMPAT_H

#include <ntcompat/windows.h>
#include <ntcompat/crt.h>
#include <ntcompat/errno.h>

#ifndef NTCOMPAT_EXPORTS
#define NTCOMPAT_EXPORTS __declspec(dllimport)
#endif

NTCOMPAT_EXPORTS HANDLE NT4_GetCurrentThreadHandle(void);

#endif
