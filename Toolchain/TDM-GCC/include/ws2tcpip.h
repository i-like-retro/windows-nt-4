/*
 * ws2tcpip.h
 *
 * TCP/IP specific extensions to the Windows Sockets API v2
 *
 *
 * $Id: ws2tcpip.h,v 0765242a471d 2020/07/05 21:32:09 keith $
 *
 * Contributed by Danny Smith <dannysmith@users.sourceforge.net>
 * Copyright (C) 2001-2003, 2005-2008, 2020, MinGW.org Project
 *
 * Portions Copyright (c) 1980, 1983, 1988, 1993
 * The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef _WS2TCPIP_H
#pragma GCC system_header
#define _WS2TCPIP_H

#if defined _WINSOCK_H && ! defined _WINSOCK2_H
#error "ws2tcpip.h is not compatible with winsock.h; include winsock2.h instead."
#endif

#include <winsock2.h>

_BEGIN_C_DECLS

/* The IP_* macros are also defined in winsock.h, but some values are different there.
 * The values defined in winsock.h for 1.1 and used in wsock32.dll are consistent with
 * the original values Steve Deering defined in his document, "IP Multicast Extensions
 * for 4.3BSD UNIX related systems (MULTICAST 1.2 Release)".  However, these conflict
 * with the definitions for some IPPROTO_IP level socket options already assigned by
 * BSD, so Berkeley changed all the values by adding 7.  WinSock2 (ws2_32.dll) uses
 * the BSD 4.4 compatible values defined here.
 *
 * See also: msdn kb article Q257460
 * http://support.microsoft.com/support/kb/articles/Q257/4/60.asp
 */
#define IP_OPTIONS			 1	/* unchanged from WinSock v1.1 */
#define IP_HDRINCL			 2	/* unchanged from WinSock v1.1 */
#define IP_TOS				 3	/* had value 8 in WinSock v1.1 */
#define IP_TTL				 4	/* had value 7 in WinSock v1.1 */
#define IP_MULTICAST_IF 		 9	/* had value 2 in WinSock v1.1 */
#define IP_MULTICAST_TTL		10	/* had value 3 in WinSock v1.1 */
#define IP_MULTICAST_LOOP		11	/* had value 4 in WinSock v1.1 */
#define IP_ADD_MEMBERSHIP		12	/* had value 5 in WinSock v1.1 */
#define IP_DROP_MEMBERSHIP		13	/* had value 6 in WinSock v1.1 */
#define IP_DONTFRAGMENT 		14	/* had value 9 in WinSock v1.1 */
#define IP_ADD_SOURCE_MEMBERSHIP	15	/* undefined in WinSock v1.1   */
#define IP_DROP_SOURCE_MEMBERSHIP	16	/* undefined in WinSock v1.1   */
#define IP_BLOCK_SOURCE 		17	/* undefined in WinSock v1.1   */
#define IP_UNBLOCK_SOURCE		18	/* undefined in WinSock v1.1   */
#define IP_PKTINFO			19	/* undefined in WinSock v1.1   */

/* As with BSD implementation, IPPROTO_IPV6 level socket options have
 * the same values as their IPv4 counterparts.
 */
#define IPV6_UNICAST_HOPS			 4
#define IPV6_MULTICAST_IF			 9
#define IPV6_MULTICAST_HOPS			10
#define IPV6_MULTICAST_LOOP			11
#define IPV6_ADD_MEMBERSHIP			12
#define IPV6_DROP_MEMBERSHIP			13
#define IPV6_JOIN_GROUP 		IPV6_ADD_MEMBERSHIP
#define IPV6_LEAVE_GROUP		IPV6_DROP_MEMBERSHIP
#define IPV6_PKTINFO				19

#define IP_DEFAULT_MULTICAST_TTL		 1
#define IP_DEFAULT_MULTICAST_LOOP		 1
#define IP_MAX_MEMBERSHIPS			20

#define TCP_EXPEDITED_1122			 2

#define UDP_NOCHECKSUM				 1

/* INTERFACE_INFO iiFlags
 */
#define IFF_UP					 1
#define IFF_BROADCAST				 2
#define IFF_LOOPBACK				 4
#define IFF_POINTTOPOINT			 8
#define IFF_MULTICAST				16

#define SIO_GET_INTERFACE_LIST	     _IOR('t', 127, u_long)

/* Minimum buffer sizes required, to store any arbitrary IPv4,
 * or IPv6 address, respectively, in string format.
 */
#define INET_ADDRSTRLEN 			16
#define INET6_ADDRSTRLEN			46

/* Constants for use when calling the getnameinfo() function;
 * first, the recommended lengths for host-name and service-name
 * return data buffers...
 */
#define NI_MAXHOST			      1025
#define NI_MAXSERV				32

/* ...and also, symbolic names for the flags argument bits.
 */
#define NI_NOFQDN 	0x01
#define NI_NUMERICHOST	0x02
#define NI_NAMEREQD	0x04
#define NI_NUMERICSERV	0x08
#define NI_DGRAM	0x10

/* Options which may be passed, as inclusive-or bit flags, in
 * the hints->ai_flags argument, to getaddrinfo()
 */
#define AI_PASSIVE				 1
#define AI_CANONNAME				 2
#define AI_NUMERICHOST				 4

/* Error codes which may be returned by the getaddrinfo() and
 * getnameinfo() functions.  These are the symbolic names which
 * conform to IETF convention; each represents, and is mapped
 * to, a corresponding WinSock error condition.
 */
#define EAI_AGAIN			WSATRY_AGAIN
#define EAI_BADFLAGS			WSAEINVAL
#define EAI_FAIL			WSANO_RECOVERY
#define EAI_FAMILY			WSAEAFNOSUPPORT
#define EAI_MEMORY			WSA_NOT_ENOUGH_MEMORY
#define EAI_NODATA			WSANO_DATA
#define EAI_NONAME			WSAHOST_NOT_FOUND
#define EAI_OVERFLOW			WSAENAMETOOLONG
#define EAI_SERVICE			WSATYPE_NOT_FOUND
#define EAI_SOCKTYPE			WSAESOCKTNOSUPPORT
#define EAI_SYSTEM			WSASYSCALLFAILURE

/* The ip_mreq structure is also required by WinSock v1.1, for
 * which case it is declared in <winsock.h>, since the definition
 * here is visible only to WinSock v2 applications which include
 * this <ws2tcpip.h> header file.
 */
struct ip_mreq
{ struct in_addr	 imr_multiaddr;
  struct in_addr	 imr_interface;
};

struct ip_mreq_source
{ struct in_addr	 imr_multiaddr;
  struct in_addr	 imr_sourceaddr;
  struct in_addr	 imr_interface;
};

struct ip_msfilter
{ struct in_addr	 imsf_multiaddr;
  struct in_addr	 imsf_interface;
  u_long		 imsf_fmode;
  u_long		 imsf_numsrc;
  struct in_addr	 imsf_slist[1];
};

#define IP_MSFILTER_SIZE(numsrc)				\
 ( sizeof( struct ip_msfilter ) - sizeof( struct in_addr )	\
   + (numsrc) * sizeof( struct in_addr )			\
 )

typedef
struct in_pktinfo
{ IN_ADDR		 ipi_addr;
  UINT			 ipi_ifindex;
} IN_PKTINFO;

/* IPv6
 *
 * This implementation requires WinXP or .NET Server or use of add-on
 * IPv6 stacks on WinNT4 and Win2K legacy platforms.
 *
 * The following in6_addr definition is based on the example given in
 * RFC 3493, with stdint types replaced by BSD types.  For now, we use
 * these field names until there is some consistency in MS docs; Within
 * this file, we only use the in6_addr structure start address, with
 * casts to get the right offsets when testing addresses.
 */
struct in6_addr
{ union
  { u_char		 _S6_u8[16];
    u_short		 _S6_u16[8];
    u_long		 _S6_u32[4];
  }			_S6_un;
# define s6_addr	_S6_un._S6_u8	/* RFC 3493 standard name */
};

/* The following are GLIBC specific field names...
 */
#define s6_addr16	_S6_un._S6_u16
#define s6_addr32	_S6_un._S6_u32

/* ...while these may be found in some Microsoft code.
 */
#define in_addr6	 in6_addr
#define _s6_bytes	_S6_un._S6_u8
#define _s6_words	_S6_un._S6_u16

typedef struct in6_addr IN6_ADDR,  *PIN6_ADDR, *LPIN6_ADDR;

typedef
struct sockaddr_in6
{ short 		 sin6_family;	/* AF_INET6 */
  u_short		 sin6_port; 	/* transport layer port # */
  u_long		 sin6_flowinfo; /* IPv6 traffic class & flow info */
  struct in6_addr	 sin6_addr;	/* IPv6 address */
  u_long		 sin6_scope_id;	/* set of interfaces for a scope */
} SOCKADDR_IN6, *PSOCKADDR_IN6, *LPSOCKADDR_IN6;

extern const struct in6_addr in6addr_any;
extern const struct in6_addr in6addr_loopback;

#define IN6ADDR_ANY_INIT        { 0 }
#define IN6ADDR_LOOPBACK_INIT   { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }


/* IPv6 Address Testing Macros
 *
 * Each of these macros, as specified in RFC 3493, exhibits semantics
 * of a function taking a "const struct in6_addr *" argument; (static
 * inline functions would allow type checking, but RFC 3493 stipulates
 * that they are macros, so we implement them as such).
 *
 * Note that IPv6 addresses are expressed in big-endian byte order.
 * Host-native addressing is little-endian; thus, we implement each of
 * these macros to interpret its argument as a byte array, stored in
 * big-endian order, (reversing byte order throughout the quad-byte
 * representation of the array).
 *
 * Further note that, while each of the following macros is named as
 * specified in RFC 3493, the bit patterns, on which classification of
 * the addresses is based, are specified in RFC 4291.
 */
#define IN6_IS_ADDR_UNSPECIFIED(_addr)				\
/* All 16 bytes of the IPv6 unspecified (i.e. the wildcard)	\
 * address MUST be zero.					\
 */								\
 (    (((const u_long *)(_addr))[0] == 0)			\
   && (((const u_long *)(_addr))[1] == 0)			\
   && (((const u_long *)(_addr))[2] == 0)			\
   && (((const u_long *)(_addr))[3] == 0)			\
 )

#define IN6_IS_ADDR_LOOPBACK(_addr)				\
/* Loopback address is ::1; for big-endian storage, the least	\
 * significant byte, with value one, must be moved to the most	\
 * significant byte of the most significant quad-byte, giving	\
 * it an effective value of 0x01000000				\
 */								\
 (    (((const u_long *)(_addr))[0] == 0)			\
   && (((const u_long *)(_addr))[1] == 0)			\
   && (((const u_long *)(_addr))[2] == 0)			\
   && (((const u_long *)(_addr))[3] == 0x01000000)		\
 )

#define IN6_IS_ADDR_LINKLOCAL(_addr)				\
/* IPv6 link-local addresses are identified by having their	\
 * ten most significant bits equal to 1111111010b		\
 */								\
 (    (((const u_char *)(_addr))[0] == 0xfe)			\
   && ((((const u_char *)(_addr))[1] & 0xc0) == 0x80)		\
 )

#define IN6_IS_ADDR_SITELOCAL(_addr)				\
/* RFC 4291 says that IPv6 site-local addresses are obsolete,	\
 * and forbids their use in new applications; nonetheless they	\
 * are still permitted in pre-existing applications, in which	\
 * they may be indentified by having their most significant	\
 * ten bits equal to 1111111011b				\
 */								\
 (    (((const u_char *)(_addr))[0] == 0xfe)			\
   && ((((const u_char *)(_addr))[1] & 0xc0) == 0xc0)		\
 )

#define IN6_IS_ADDR_V4MAPPED(_addr)				\
/* An IPv4 mapped address reserves the least significant four	\
 * bytes, (most significant in big-endian equivalent storage),	\
 * for the IPv4 address; the immediately adjacent two bytes	\
 * MUST be 0xffff, with the remaining ten bytes all zero.	\
 */								\
 (    (((const u_long *)(_addr))[0] == 0)			\
   && (((const u_long *)(_addr))[1] == 0)			\
   && (((const u_long *)(_addr))[2] == 0xffff0000)		\
 )

#define IN6_IS_ADDR_V4COMPAT(_addr)				\
/* IPv4 compatible address format is effectively obsolete;	\
 * nonetheless, it may be recognized as being the same as the	\
 * IPv4 mapped format, but with 0x0000 in place of the 0xffff	\
 * byte pair, and the IPv4 address represented being neither	\
 * 0.0.0.0, nor 0.0.0.1, (again noting the byte reversal for	\
 * big-endian storage order).					\
 */								\
 (    (((const u_long *)(_addr))[0] == 0)			\
   && (((const u_long *)(_addr))[1] == 0)			\
   && (((const u_long *)(_addr))[2] == 0)			\
   && (((const u_long *)(_addr))[3] != 0)			\
   && (((const u_long *)(_addr))[3] != 0x01000000)		\
 )

#define IN6_IS_ADDR_MULTICAST(_addr)				\
/* Any IPv6 address in which the most significant byte, (least	\
 * significant in host storage order), has a value of 0xff, is	\
 * a multicast address. 					\
 */								\
 (((const u_char *)(_addr))[0] == 0xff)

/* IPv6 multicast addresses may be further classified into
 * "scope" groups, based on the value of the low order four
 * bits of the second most significant byte; as above, the
 * following macro names are specified in RFC 3493, but the
 * scope indices are specified in RFC 4291.
 */
#define IN6_IS_ADDR_MC_NODELOCAL(_addr) 			\
/* IPv6 multicast addresses with a scope index of one refer	\
 * only to interfaces of the originating host node; (this is	\
 * a multicast equivalent to the loopback address).		\
 */								\
 ( IN6_IS_ADDR_MULTICAST(_addr) 				\
   && ((((const u_char *)(_addr))[1] & 0xf) == 0x1)		\
 )

#define IN6_IS_ADDR_MC_LINKLOCAL(_addr) 			\
/* IPv6 multicast addresses with a scope index of two refer	\
 * to interfaces on the same subnet as the originating host.	\
 */								\
 ( IN6_IS_ADDR_MULTICAST(_addr) 				\
   && ((((const u_char *)(_addr))[1] & 0xf) == 0x2)		\
 )

#define IN6_IS_ADDR_MC_SITELOCAL(_addr) 			\
/* IPv6 multicast addresses with a scope index of five refer	\
 * to interfaces, possibly spanning multiple subnets, but all	\
 * within the confines of a single geographical location.	\
 */								\
 ( IN6_IS_ADDR_MULTICAST(_addr) 				\
   && ((((const u_char *)(_addr))[1] & 0xf) == 0x5)		\
 )

#define IN6_IS_ADDR_MC_ORGLOCAL(_addr)				\
/* IPv6 multicast addresses with a scope index of eight refer	\
 * to interfaces which are geographically distributed across	\
 * multiple sites, all belonging to a single organization.	\
 */								\
 ( IN6_IS_ADDR_MULTICAST(_addr) 				\
   && ((((const u_char *)(_addr))[1] & 0xf) == 0x8)		\
 )

#define IN6_IS_ADDR_MC_GLOBAL(_addr)				\
/* IPv6 multicast addresses with a scope index of fourteen	\
 * refer to interfaces which may be globally distributed over	\
 * the public internet. 					\
 */								\
 ( IN6_IS_ADDR_MULTICAST(_addr) 				\
   && ((((const u_char *)(_addr))[1] & 0xf) == 0xe)		\
 )

/* RFC 3542 augments the preceding set of address testing macros, by the
 * addition of one more, intended for use in "advanced" applications; it
 * is semantically equivalent to a function with prototype:
 *
 *   int IN6_ARE_ADDR_EQUAL
 *   ( const struct in6_addr *, const struct in6_addr * )
 *
 * and returns non-zero if its two IPv6 address arguments are the same,
 * or zero, if they differ.
 */
#define IN6_ARE_ADDR_EQUAL(_addr1, _addr2)			\
 ( memcmp( (void *)(_addr1), (void *)(_addr2),			\
	    sizeof( struct in6_addr )				\
	 ) == 0							\
 )

typedef int socklen_t;

typedef
struct ipv6_mreq
{ struct in6_addr	 ipv6mr_multiaddr;
  unsigned int		 ipv6mr_interface;
} IPV6_MREQ;

typedef
struct in6_pktinfo
{ IN6_ADDR		 ipi6_addr;
  UINT			 ipi6_ifindex;
} IN6_PKTINFO;

struct addrinfo
{ int			 ai_flags;
  int			 ai_family;
  int			 ai_socktype;
  int			 ai_protocol;
  size_t		 ai_addrlen;
  char			*ai_canonname;
  struct sockaddr	*ai_addr;
  struct addrinfo	*ai_next;
};

#if _WIN32_WINNT >= _WIN32_WINNT_WINXP && ! defined _WSPIAPI_H
/* The following three functions are required by RFC 3493, (formerly
 * RFC 2553), but MS-Windows did not support them natively, prior to
 * Win-XP.  On earlier Windows versions, they may be supported, (but
 * without IPv6 support), by including <wspiapi.h>; (on Win-2K, IPv6
 * support may be attainable, if the IPv6 Technology Preview kit has
 * been installed, and the application is linked with WSHIP6.DLL).
 *
 * We expose these declarations only if we are compiling for Win-XP
 * or later, and then only if <wspiapi.h> has not been included.
 */
WSAAPI int getaddrinfo
(const char *, const char *, const struct addrinfo *, struct addrinfo **);

WSAAPI int getnameinfo
(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t, int);

WSAAPI void freeaddrinfo (struct addrinfo *);
#endif

#define gai_strerror __AW_SUFFIXED__( gai_strerror )

static __inline__
char *gai_strerrorA (int ecode)
{
  static char message[1024+1];
  DWORD Flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
    | FORMAT_MESSAGE_MAX_WIDTH_MASK;
  DWORD LanguageId = MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT);
  FormatMessageA (Flags, NULL, ecode, LanguageId, message, 1024, NULL);
  return message;
}

static __inline__
WCHAR *gai_strerrorW (int ecode)
{
  static WCHAR message[1024+1];
  DWORD Flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS
    | FORMAT_MESSAGE_MAX_WIDTH_MASK;
  DWORD LanguageId = MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT);
  FormatMessageW (Flags, NULL, ecode, LanguageId, message, 1024, NULL);
  return message;
}

/* Some older IPv4/IPv6 compatibility stuff */

/* This struct lacks sin6_scope_id; retained for use in sockaddr_gen */
struct sockaddr_in6_old
{ short 			sin6_family;
  u_short			sin6_port;
  u_long			sin6_flowinfo;
  struct in6_addr		sin6_addr;
};

typedef
union sockaddr_gen
{ struct sockaddr		Address;
  struct sockaddr_in		AddressIn;
  struct sockaddr_in6_old	AddressIn6;
} sockaddr_gen;

typedef
struct _INTERFACE_INFO
{ u_long			iiFlags;
  sockaddr_gen			iiAddress;
  sockaddr_gen			iiBroadcastAddress;
  sockaddr_gen			iiNetmask;
} INTERFACE_INFO, *LPINTERFACE_INFO;

#if 0
/* The definition above may cause problems on WinNT4, prior to
 * service pack 4.  To work around this, if necessary, include
 * the following typedef and struct definition, and
 *
 *   #define INTERFACE_INFO  OLD_INTERFACE_INFO
 *
 * See FIX: WSAIoctl SIO_GET_INTERFACE_LIST Option Problem
 * (Q181520) in MSDN KB.  Do note, however, that exposure of
 * the old definition may cause problems on newer variants of
 * WinNT, and on WinXP and later.
 */
typedef struct _OLD_INTERFACE_INFO
{ u_long			iiFlags;
  struct sockaddr		iiAddress;
  struct sockaddr		iiBroadcastAddress;
  struct sockaddr		iiNetmask;
} OLD_INTERFACE_INFO;
#endif

_END_C_DECLS

#endif	/* !_WS2TCPIP_H: $RCSfile: ws2tcpip.h,v $: end of file */
