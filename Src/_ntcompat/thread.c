#define NTCOMPAT_EXPORTS __declspec(dllexport)
#include "ntcompat/ntcompat.h"

__declspec(dllexport) HANDLE NT4_GetCurrentThreadHandle(void)
{
    HANDLE converted = NULL;
    HANDLE hProcess = GetCurrentProcess();

    if (!DuplicateHandle(hProcess, GetCurrentThread(), hProcess,
            &converted, 0, FALSE, DUPLICATE_SAME_ACCESS))
        return NULL;

    return converted;
}
