#ifndef NTCOMPAT_WINDOWS_H
#define NTCOMPAT_WINDOWS_H

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <errno.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef getaddrinfo
#undef getaddrinfo
#endif

#ifdef freeaddrinfo
#undef freeaddrinfo
#endif

#ifdef getnameinfo
#undef getnameinfo
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

#ifndef SECURITY_MAX_SID_SIZE
#define SECURITY_MAX_SID_SIZE 68
#endif

#ifndef _In_
#define _In_
#endif

#ifndef _Out_
#define _Out_
#endif

#ifndef _In_opt_
#define _In_opt_
#endif

#ifndef _Inout_
#define _Inout_
#endif

#ifndef _Inout_opt_
#define _Inout_opt_
#endif

#ifndef _Out_opt_
#define _Out_opt_
#endif

#ifndef _Outptr_
#define _Outptr_
#endif

#ifndef _Outptr_opt_
#define _Outptr_opt_
#endif

#ifndef WSAID_ACCEPTEX
#define WSAID_ACCEPTEX {0xb5367df1,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#endif
#ifndef WSAID_CONNECTEX
#define WSAID_CONNECTEX {0x25a207b9,0xddf3,0x4660,{0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}}
#endif
#ifndef WSAID_GETACCEPTEXSOCKADDRS
#define WSAID_GETACCEPTEXSOCKADDRS {0xb5367df2,0xcbac,0x11cf,{0x95,0xca,0x00,0x80,0x5f,0x48,0xa1,0x92}}
#endif

#ifndef SO_UPDATE_CONNECT_CONTEXT
#define SO_UPDATE_CONNECT_CONTEXT 0x7010
#endif

#ifndef POLLRDNORM
#define POLLRDNORM 0x0100
#endif

#ifndef POLLRDBAND
#define POLLRDBAND 0x0200
#endif

#ifndef POLLIN
#define POLLIN (POLLRDNORM | POLLRDBAND)
#endif

#ifndef POLLWRNORM
#define POLLWRNORM 0x0010
#endif

#ifndef POLLOUT
#define POLLOUT (POLLWRNORM)
#endif

#ifndef POLLERR
#define POLLERR 0x0001
#endif

#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif

typedef BOOL (APIENTRY* PFNACCEPTEXPROC)
    (SOCKET, SOCKET, PVOID, DWORD, DWORD, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL (APIENTRY* PFNCONNECTEXPROC)
    (SOCKET, const struct sockaddr*, int, PVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef VOID (APIENTRY* PFNGETACCEPTEXSOCKADDRSPROC)
    (PVOID, DWORD, DWORD, DWORD, struct sockaddr**, LPINT, struct sockaddr**, LPINT);

#ifndef LPFN_ACCEPTEX
#define LPFN_ACCEPTEX PFNACCEPTEXPROC
#endif

#ifndef LPFN_CONNECTEX
#define LPFN_CONNECTEX PFNCONNECTEXPROC
#endif

#ifndef LPFN_GETACCEPTEXSOCKADDRS
#define LPFN_GETACCEPTEXSOCKADDRS PFNGETACCEPTEXSOCKADDRSPROC
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

#ifndef _countof
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(array) (sizeof(array) / sizeof(array[0]))
#endif

#ifndef __ascii_iswalpha
#define __ascii_iswalpha(c)  ( ('A' <= (c) && (c) <= 'Z') || ( 'a' <= (c) && (c) <= 'z'))
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
