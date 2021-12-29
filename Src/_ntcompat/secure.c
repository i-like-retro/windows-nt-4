#define NTCOMPAT_EXPORTS __declspec(dllexport)
#include "ntcompat/ntcompat.h"
#include <stdio.h>

__declspec(dllexport) size_t strlcpy(char* dst, const char* src, size_t size)
{
    size_t len;

    if (size == 0)
        return 0;

    --size;

    len = strlen(src);
    if (len > size)
        len = size;

    memcpy(dst, src, len);
    dst[len] = 0;

    return len;
}

__declspec(dllexport) size_t strlcat(char* dst, const char* src, size_t size)
{
    size_t len = strlen(dst);
    if (size <= len)
        return 0;

    return strlcpy(dst + len, src, size - len);
}

__declspec(dllexport) errno_t memcpy_s(void* dest, size_t destsz, const void* src, size_t count)
{
    errno_t result = 0;
    if (count > destsz)
        return -1;
    memcpy(dest, src, count);
    return 0;
}

__declspec(dllexport) errno_t memmove_s(void* dest, size_t destsz, const void* src, size_t count)
{
    errno_t result = 0;
    if (count > destsz)
        return -1;
    memmove(dest, src, count);
    return 0;
}

__declspec(dllexport) errno_t strcpy_s(char* dest, size_t destsz, const char* src)
{
    size_t len;

    if (destsz == 0)
        return -1;

    --destsz;

    len = strlen(src);
    if (len > destsz) {
        dest[0] = 0;
        return -1;
    }

    memcpy(dest, src, len);
    dest[len] = 0;

    return 0;
}

__declspec(dllexport) errno_t strcat_s(char* dest, size_t destsz, const char* src)
{
    size_t len = strlen(dest);
    if (destsz <= len)
        return -1;

    return strcpy_s(dest + len, destsz - len, src);
}

__declspec(dllexport) errno_t strncpy_s(char* dest, size_t destsz, const char* src, size_t count)
{
    size_t len = strlen(src);
    if (len >= destsz) {
        if (destsz > 0)
            dest[0] = 0;
        return -1;
    }
    memcpy(dest, src, len);
    dest[len] = 0;
    return 0;
}

__declspec(dllexport) errno_t wcscpy_s(wchar_t* dest, size_t destsz, const wchar_t* src)
{
    size_t len;

    if (destsz == 0)
        return -1;

    --destsz;

    len = wcslen(src);
    if (len > destsz) {
        dest[0] = 0;
        return -1;
    }

    memcpy(dest, src, len * sizeof(wchar_t));
    dest[len] = 0;

    return 0;
}

__declspec(dllexport) errno_t wcscat_s(wchar_t* dest, size_t destsz, const wchar_t* src)
{
    size_t len = wcslen(dest);
    if (destsz <= len)
        return -1;

    return wcscpy_s(dest + len, destsz - len, src);
}

__declspec(dllexport) int _snprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, ...)
{
    va_list args;
    int len;

    if (sizeOfBuffer == 0)
        return 0;

    va_start(args, format);
    len = _vsnprintf(buffer, sizeOfBuffer, format, args);
    va_end(args);

    if (len < 0) {
        buffer[0] = 0;
        return -1;
    }

    if ((size_t)len >= sizeOfBuffer) {
        if (count == _TRUNCATE)
            return sizeOfBuffer - 1;
        if ((int)count < len && count < sizeOfBuffer) {
            buffer[count] = 0;
            return count;
        }
        buffer[0] = 0;
        errno = ERANGE;
        return -1;
    }

    if (count != _TRUNCATE && len > (int)count) {
        buffer[count] = 0;
        len = (int)count;
    }

    return len;
}

__declspec(dllexport) int vsnprintf_s(char* buffer, size_t sizeOfBuffer, size_t count, const char* format, va_list args)
{
    int len;

    if (sizeOfBuffer == 0)
        return 0;

    len = _vsnprintf(buffer, sizeOfBuffer, format, args);
    if (len < 0) {
        buffer[0] = 0;
        return -1;
    }

    if ((size_t)len >= sizeOfBuffer) {
        if (count == _TRUNCATE)
            return sizeOfBuffer - 1;
        if ((int)count < len && count < sizeOfBuffer) {
            buffer[count] = 0;
            return count;
        }
        buffer[0] = 0;
        errno = ERANGE;
        return -1;
    }

    if (count != _TRUNCATE && len > (int)count) {
        buffer[count] = 0;
        len = (int)count;
    }

    return len;
}
