#ifndef NTCOMPAT_CRT_H
#define NTCOMPAT_CRT_H

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define errno_t int

#ifndef _TRUNCATE
#define _TRUNCATE ((size_t)-1)
#endif

#ifndef NTCOMPAT_EXPORTS
#define NTCOMPAT_EXPORTS __declspec(dllimport)
#endif

NTCOMPAT_EXPORTS size_t strlcat(char* dst, const char* src, size_t size);
NTCOMPAT_EXPORTS size_t strlcpy(char* dst, const char* src, size_t size);

NTCOMPAT_EXPORTS errno_t memcpy_s(void* dest, size_t destsz, const void* src, size_t count);
NTCOMPAT_EXPORTS errno_t memmove_s(void* dest, size_t destsz, const void* src, size_t count);

NTCOMPAT_EXPORTS errno_t strcpy_s(char* dest, size_t destsz, const char* src);
NTCOMPAT_EXPORTS errno_t strcat_s(char* dest, size_t destsz, const char* src);
NTCOMPAT_EXPORTS errno_t strncpy_s(char* dest, size_t destsz, const char* src, size_t count);

NTCOMPAT_EXPORTS errno_t wcscpy_s(wchar_t* dest, size_t dest_size, const wchar_t* src);
NTCOMPAT_EXPORTS errno_t wcscat_s(wchar_t* strDestination, size_t numberOfElements, const wchar_t* strSource);

NTCOMPAT_EXPORTS int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...);
NTCOMPAT_EXPORTS int vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list args);

#ifdef __cplusplus
}
#endif

#endif
