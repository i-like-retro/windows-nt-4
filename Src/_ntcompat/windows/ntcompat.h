#ifndef WINDOWS_NTCOMPAT_H
#define WINDOWS_NTCOMPAT_H

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef AI_PASSIVE
#define AI_PASSIVE 1
#endif

#ifndef AI_CANONNAME
#define AI_CANONNAME 2
#endif

#ifndef AI_NUMERICHOST
#define AI_NUMERICHOST 4
#endif

#ifndef EAI_AGAIN
#define EAI_AGAIN WSATRY_AGAIN
#endif

#ifndef EAI_BADFLAGS
#define EAI_BADFLAGS WSAEINVAL
#endif

#ifndef EAI_FAIL
#define EAI_FAIL WSANO_RECOVERY
#endif

#ifndef EAI_FAMILY
#define EAI_FAMILY WSAEAFNOSUPPORT
#endif

#ifndef EAI_MEMORY
#define EAI_MEMORY WSA_NOT_ENOUGH_MEMORY
#endif

#ifndef EAI_NONAME
#define EAI_NONAME WSAHOST_NOT_FOUND
#endif

#ifndef EAI_OVERFLOW
#define EAI_OVERFLOW WSAENAMETOOLONG
#endif

#ifndef EAI_SERVICE
#define EAI_SERVICE WSATYPE_NOT_FOUND
#endif

#ifndef EAI_SOCKTYPE
#define EAI_SOCKTYPE WSAESOCKTNOSUPPORT
#endif

#ifndef EAI_SYSTEM
#define EAI_SYSTEM WSASYSCALLFAILURE
#endif

#ifndef NI_NOFQDN
#define NI_NOFQDN 0x01
#endif

#ifndef NI_NUMERICHOST
#define NI_NUMERICHOST 0x02
#endif

#ifndef NI_NAMEREQD
#define NI_NAMEREQD 0x04
#endif

#ifndef NI_NUMERICSERV
#define NI_NUMERICSERV 0x08
#endif

#ifndef NI_DGRAM
#define NI_DGRAM 0x10
#endif

#if defined(_MSC_VER) && _MSC_VER == 1200
typedef int socklen_t;
struct addrinfo
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    char* ai_canonname;
    struct sockaddr* ai_addr;
    struct addrinfo* ai_next;
};
#endif

#ifndef NTCOMPAT_EXPORTS
#define NTCOMPAT_EXPORTS __declspec(dllimport)
#endif

NTCOMPAT_EXPORTS BOOL WINAPI CreateHardLinkA(LPCSTR lpFileName, LPCSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);
NTCOMPAT_EXPORTS BOOL WINAPI CreateHardLinkW(LPCWSTR lpFileName, LPCWSTR lpExistingFileName,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes);

#ifdef UNICODE
#define CreateHardLink CreateHardLinkW
#else
#define CreateHardLink CreateHardLinkA
#endif

NTCOMPAT_EXPORTS int WINAPI getaddrinfo(const char* node, const char* service,
    const struct addrinfo* hints, struct addrinfo** response_list);

NTCOMPAT_EXPORTS void WINAPI freeaddrinfo(struct addrinfo* ai_chain);

NTCOMPAT_EXPORTS int WINAPI getnameinfo(const struct sockaddr* addr,
    socklen_t len, char* node, socklen_t nodelen, char* service,
    socklen_t servlen, int flags);

#ifdef __cplusplus
}
#endif

#endif
