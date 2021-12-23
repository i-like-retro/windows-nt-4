/*
 * wspiapi.h
 *
 * Implementation for pre-WinXP getaddrinfo() and getnameinfo() APIs.
 *
 *
 * $Id: wspiapi.h,v 0765242a471d 2020/07/05 21:32:09 keith $
 *
 * Written by Keith Marshall <keith@users.osdn.me>
 * Copyright (C) 2020, MinGW.org Project
 *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 *
 * Author's Note:
 * I have written this implementation of <wspiapi.h> from scratch, based
 * entirely on hints from publicly visible Microsoft documentation, my own
 * interpretation of IETF's RFC 3493, the corresponding POSIX.1 specification,
 * and observation of the behaviour of example code from Microsoft's publicly
 * visible documentation, which I have adapted to run on my GNU/Linux host.
 * I have never seen code for Microsoft's <wspiapi.h> implementation; nor
 * have I copied code from any other implementation, however licensed.
 *
 * This implementation servs as a fall-back for use on those legacy versions
 * of Windows which do not support the getaddrinfo() and getnameinfo() APIs,
 * through WS2_32.DLL on WinXP and later, or through WSHIP6.DLL on Win2K
 * (with the IPv6 Preview Kit).  Since such legacy Windows versions do not
 * otherwise support IPv6, there is no attempt to support the AF_INET6
 * address family; AF_INET is supported, as is AF_UNSPEC, but only to
 * the extent that it is interpreted as AF_INET alone.
 *
 * It should be noted that, as described in Microsoft's documentation, the
 * fall-back code is implemented in the form of in-line functions.  This
 * has a potentially disadvantageous side effect, in that every translation
 * unit which calls any of the fall-back functions will incur a substantial
 * overhead of potentially duplicated fall-back code.  To mitigate this, it
 * is recommended that all such calls be encapsulated within one translation
 * unit, and exposed globally through public wrapper functions; an example
 * of this technique may be found in the gcc/ada/socket.c implementation
 * within recent GCC sources, wherein the public API is exposed via the
 * __gnat_getaddrinfo(), __gnat_freeaddrinfo(), and __gnat_getnameinfo()
 * function wrappers.
 *
 */
#ifndef _WSPIAPI_H
#pragma GCC system_header
#define _WSPIAPI_H

#include <stdio.h>
#include <ws2tcpip.h>

/* Symbolic constants are useful as wild-card identifiers for socket types
 * and protocols, but these are non-standard; we don't want to pollute the
 * public namespace, so we add the reserved __WSPIAPI_ prefix for these.
 */
#define __WSPIAPI_SOCK_ANY     0
#define __WSPIAPI_IPPROTO_ANY  0

_BEGIN_C_DECLS

/* __wspiapi_errout(): a helper to assign a WSA error code, via the
 * WSASetLastError() function, and also return it for immediate use
 * as the exit status for any wspiapi function.
 */
static __inline__ __attribute__((__always_inline__))
int __wspiapi_errout( int status ){ WSASetLastError( status ); return status; }

/* __wspiapi_syserrout(): variation on __wspiapi_errout(), returning
 * EAI_SYSTEM, after assigning a specific system errno value.
 */
static __inline__
__attribute__((__always_inline__))
int __wspiapi_syserrout( int errcode )
{ errno = errcode; return __wspiapi_errout( EAI_SYSTEM ); }

/* enum __wspiapi: generate symbolic names for the entry point indices
 * within the wspapi function reference dictionary.
 */
enum __wspiapi
{ __WSPIAPI_FREEADDRINFO__,
  __WSPIAPI_GETADDRINFO__,
  __WSPIAPI_GETNAMEINFO__
};

/* __wspiapi_t: a private use data type, describing the structure of
 * each individual function reference within the dictionary.
 */
typedef struct
{ const char	*name;
  void		*entry;
} __wspiapi_t;

/* __wspiapi_reference(): a helper function to retrieve a function
 * entry point address, by indexed function name look-up within the
 * reference dictionary.
 */
static __inline__ __attribute__((__always_inline__))
__wspiapi_t *__wspiapi_reference( enum __wspiapi index )
{
  /* The dictionary, itself, is statically encapsulated within this
   * helper function, (and thus, is not publicly visible).  Note that,
   * initially, all entry point vectors are specified as (void *)(-1),
   * and will be fixed-up on first call to any related function.
   */
  static __wspiapi_t dictionary[] =
  { { "freeaddrinfo", (void *)(-1) },
    { "getaddrinfo",  (void *)(-1) },
    { "getnameinfo",  (void *)(-1) }
  };

  /* The return value is simply a pointer to the dictionary entry
   * corresponding to the specified index.
   */
  return dictionary + index;
}

/* __wspiapi_set_entry(): helper function to perform the dictionary
 * fix-up, for a single function entry point reference, as described
 * for the __wspiapi_reference() function above.  Note that this is
 * called, only after a candidate DLL, which may be expected to
 * provide the associated function, has been identified.
 */
static __inline__ __attribute__((__always_inline__))
void __wspiapi_set_entry( HMODULE dll, __wspiapi_t *api )
{ api->entry = (void *)(GetProcAddress( dll, api->name )); }

/* __wspiapi_module_handle(): helper to obtain a module handle for
 * a candidate DLL, which is expected to provide all of the wspiapi
 * functions, based on it satisfying one dictionary reference.
 */
static __inline__
HMODULE __wspiapi_module_handle
( char *path, char *subst, const char *name, __wspiapi_t *api )
{
  HMODULE ref;

  /* "subst" points to the offset, within the "path" buffer, at which
   * the system directory path name ends, and the DLL file name should
   * be inserted; append the specified DLL file "name", and attempt to
   * load a system DLL of that name.
   */
  strcpy( subst, name );
  if( (ref = LoadLibraryA( path )) != NULL )
  {
    /* We successfully obtained a reference handle for the named DLL;
     * check that we can also obtain an entry point reference, within
     * this DLL, for the specified wspiapi function, recording the
     * result in the wspiapi reference dictionary...
     */
    if( (api->entry = (void *)(GetProcAddress( ref, api->name ))) == NULL )
    {
      /* ...and declaring no further wspiapi interest in this DLL, if
       * the requisite entry point is unavailable.
       */
      FreeLibrary( ref );
      return NULL;
    }
  }
  /* If we get to here, then we either have a valid handle for a DLL
   * which does provide the requisite wspiapi witness function, in
   * which case we return that handle, otherwise we return the NULL
   * resulting from a failed DLL load request.
   */
  return ref;
}

/* __wspiapi_lib(): a thin wrapper, through which all calls to
 * __wspiapi_module_handle() are directed; it passes a system DLL
 * search path, wherein the specified DLL name is substituted, (at
 * the substring offset specified by "subst"), requiring that the
 * designated system DLL should provide freeaddrinfo() as witness
 * that it may be a suitable candidate provider for all of those
 * functions which must otherwise be replaced by <wspiapi.h>
 * fall-back implementations.
 */
static __inline__ __attribute__((__always_inline__))
HMODULE __wspiapi_lib( char *path, char *subst, const char *name )
{
  /* This requires nothing more than appending the wspiapi dictionary
   * reference for the freeaddrinfo() function, to the arguments which
   * have already been specified, and forwarding the request onward,
   * to __wspiapi_module_handle().
   */
  return  __wspiapi_module_handle(
      path, subst, name, __wspiapi_reference(__WSPIAPI_FREEADDRINFO__)
    );
}

/* __wspiapi_entry(): helper to retrieve the entry point address for
 * a specified wspiapi function, by indexed look-up within the wspiapi
 * reference dictionary; invokes __wspiapi_lib(), as may be required,
 * to initialize the reference dictionary entries.
 */
static __inline__
void *__wspiapi_entry( enum __wspiapi index )
# define __wspiapi_call( index ) (call)(__wspiapi_entry( index ))
{
  /* First, check that the reference dictionary has been initialized,
   * using the freeaddrinfo() entry point reference as witness.
   */
  if( __wspiapi_reference(__WSPIAPI_FREEADDRINFO__)->entry == (void *)(-1) )
  {
    /* The dictionary appears to be uninitialized; set up to perform
     * entry point searches within DLLs in the system directory...
     */
    HMODULE dll;
    char sys_path[MAX_PATH], *dllname;
    dllname = sys_path + GetSystemDirectory( sys_path, MAX_PATH - 11 );
    if( dllname > sys_path )
    { /* ...then, preferring WS2_32.DLL, but accepting WSHIP6.DLL as
       * a second choice alternative, initialize the dictionary entry
       * for the freeaddrinfo() witness function...
       */
      if( ((dll = __wspiapi_lib( sys_path, dllname, "\\ws2_32" )) != NULL)
      ||  ((dll = __wspiapi_lib( sys_path, dllname, "\\wship6" )) != NULL)  )
      {
	/* ...and, when that resolves to a valid entry point address,
	 * also initialize the entry point references, using the same
	 * DLL, for the getaddrinfo() and getnameinfo() functions...
	 */
	__wspiapi_set_entry( dll, __wspiapi_reference(__WSPIAPI_GETADDRINFO__) );
	__wspiapi_set_entry( dll, __wspiapi_reference(__WSPIAPI_GETNAMEINFO__) );
	FreeLibrary( dll );
      }
      else
      { /* ...otherwise, initialize these remaining references, such
	 * that all three functions are marked as unsupported by any
	 * system DLL, and thus must be emulated, on demand, by use
	 * of <wspiapi.h> in-line code.
	 */
	__wspiapi_reference(__WSPIAPI_GETADDRINFO__)->entry = NULL;
	__wspiapi_reference(__WSPIAPI_GETNAMEINFO__)->entry = NULL;
      }
    }
  }
  /* Irrespective of whether initialization was performed on this
   * occasion, or had been performed previously, we return the entry
   * point address for the requested function, as it is now recorded
   * in the wspiapi reference dictionary.
   */
  return __wspiapi_reference( index )->entry;
}

/* __wspiapi_freeaddrinfo(): fall-back implementation for freeaddrinfo(),
 * invoked by WspiapiFreeAddrInfo(), if (and only if), there is no native
 * implementation provided in WS2_32.DLL, or by way of WSHIP6.DLL, from
 * the Win2K IPv6 Technology Preview.
 */
static __inline__
WSAAPI void __wspiapi_freeaddrinfo( struct addrinfo *ai_chain )
{
  /* This must free all addrinfo records on a single chain, which has been
   * allocated by a prior call of __wspiapi_getaddrinfo(); note that it is
   * necessary to free any associated ai_canonname records, which have been
   * independently allocated, but not any ai_addr records, as these are all
   * allocated as integral elements of the owner addrinfo record itself.
   */
  while( ai_chain != NULL )
  { struct addrinfo *next = ai_chain->ai_next;
    free( ai_chain->ai_canonname ); free( ai_chain );
    ai_chain = next;
  }
}

/* __wspiapi_addrinfo_cleanup(): a local helper function, called only by
 * __wspiapi_getaddrinfo(), to clean up any partially allocated addrinfo
 * record chain, in the event of __wspiapi_getaddrinfo() failure.
 */
static __inline__ __attribute__((__always_inline__))
struct addrinfo *__wspiapi_addrinfo_cleanup( struct addrinfo *ai_chain )
{ __wspiapi_freeaddrinfo( ai_chain ); return NULL; }

/* __wspiapi_getaddrinfo_internal(): core implementation for, and called
 * exclusively by, __wspiapi_getaddrinfo(); returns zero on success, or
 * an EAI error code, (but without updating WSAGetLastError() status),
 * on failure.
 */
static __inline__
__attribute__((__always_inline__))
int __wspiapi_getaddrinfo_internal
( const char *__restrict__ nodename, const char *__restrict__ servname,
  const struct addrinfo *__restrict__ hints, struct addrinfo **__restrict__ res
)
{ /* Either nodename, or servname, but not both, may be NULL; initially
   * assign status to reflect the invalid condition of both being NULL.
   */
  int status = EAI_NONAME;

  /* Initialize a local template, from which each allocated resultant
   * addrinfo record may be derived.
   */
  struct ai { struct addrinfo ai; struct sockaddr_in sa; } ai_data;
  memset( &ai_data, 0, sizeof( struct ai ) );

  /* Ensure that the state of the resultant addrinfo record chain will
   * be sane, in the event of early failure.
   */
  *res = NULL;

  /* The hints may be NULL, (in which case the initial template state
   * will prevail), but if specified...
   */
  if( hints != NULL )
  { /* ...then its ai_addr, ai_canonname, and ai_next fields MUST all
     * be NULL, and ai_addrlen MUST be zero.
     */
    if( (hints->ai_addr != NULL) || (hints->ai_addrlen != 0)
    ||  (hints->ai_next != NULL) || (hints->ai_canonname != NULL)  )
      return EAI_FAIL;

    /* Only the IPv4 AF_INET ai_family is supported, (but we tolerate
     * AF_UNSPEC as implying AF_INET).
     */
    switch( ai_data.ai.ai_family = hints->ai_family )
    { case AF_UNSPEC: case AF_INET: break; default: return EAI_FAMILY; }

    /* Protocol and socket type may each be specified explicitly, or
     * implicitly, (by default, or by wild-card assignment; supported
     * protocols are TCP and UDP...
     */
    switch( ai_data.ai.ai_protocol = hints->ai_protocol )
    {
      case IPPROTO_TCP:
	/* ...where an explicit choice of TCP must be associated with
	 * a stream type socket...
	 */
	switch( hints->ai_socktype )
	{ /* ...which again, may be either explicitly assigned, or
	   * implicitly deduced.
	   */
	  case SOCK_STREAM: case __WSPIAPI_SOCK_ANY:
	    ai_data.ai.ai_socktype = SOCK_STREAM;
	    break;

	  /* Any other explicit pairing results in failure.
	   */
	  default: return EAI_SOCKTYPE;
	}
	break;

      case IPPROTO_UDP:
	/* Similarly, an explicit protocol choice must be paired,
	 * explicitly or implicitly, with a datagram socket.
	 */
	switch( hints->ai_socktype )
	{ case SOCK_DGRAM: case __WSPIAPI_SOCK_ANY:
	    ai_data.ai.ai_socktype = SOCK_DGRAM;
	    break;

	  /* Once again, any other explicit pairing is invalid.
	   */
	  default: return EAI_SOCKTYPE;
	}
	break;

      case __WSPIAPI_IPPROTO_ANY:
	/* Without an explicit protocol specification, we may need
	 * to infer a match for the socket type...
	 */
	switch( ai_data.ai.ai_socktype = hints->ai_socktype )
	{
	  /* ...pairing TCP with an explicit stream socket...
	   */
	  case SOCK_STREAM:
	    ai_data.ai.ai_protocol = IPPROTO_TCP;
	    break;

	  /* ...UDP with a datagram socket...
	   */
	  case SOCK_DGRAM:
	    ai_data.ai.ai_protocol = IPPROTO_UDP;

	  /* ...or fall through, to enable generation of an addrinfo
	   * list, with all supported valid pairings.
	   */
	  case __WSPIAPI_SOCK_ANY: break;

	  /* Bail out, if any unsupported socket type is explicitly
	   * specified...
	   */
	  default: return EAI_SOCKTYPE;
	}
	break;

      /* ...and likewise, for any unsupported protocol.
       */
      default: return EAI_SOCKTYPE;
    }

    /* Simply propagate whatever flags may have been specified, but
     * defer any validation of them, until later.
     */
    ai_data.ai.ai_flags = hints->ai_flags;
  }

  /* Only the IPv4 address family is supported; whether confirmed in
   * the hints, or left unspecified, we may stipulate that now.
   */
  ai_data.ai.ai_family = ai_data.sa.sin_family = AF_INET;

  /* The servname argument may be NULL, but when it is specified, and
   * it is a non-empty string...
   */
  if( (servname != NULL) && (*servname != '\0') )
  { /* ...first try to interpret it as a textual representation of a
     * service port number...
     */
    struct servent *pi; char *brk;
    ai_data.sa.sin_port = strtoul( servname, &brk, 0 );
    if( *brk == '\0' ) pi = getservbyport( htons( ai_data.sa.sin_port ), NULL );

    /* ...or, failing that, a well known service name, which may be
     * mapped to such a port number, or else bail out.
     */
    else pi = getservbyname( servname, NULL );
    if( pi == NULL ) return EAI_SERVICE;

    /* When the servname argument has been successfully interpreted,
     * store the port number within the addrinfo template, and reset
     * the return status, to indicate that the minimal requirement
     * for at least one of servname and nodename is non-NULL.
     */
    ai_data.sa.sin_port = pi->s_port;
    status = 0;
  }

  /* Similarly, the nodename argument may be NULL, but when it is
   * specified, and is non-empty...
   */
  if( (nodename != NULL) && (*nodename != '\0') )
  { /* ...then we first attempt to interpret it as a representation,
     * in textual format, of an IPv4 address.
     */
    unsigned long *addr = (unsigned long *)(&(ai_data.sa.sin_addr));
    if( (*addr = inet_addr( nodename )) != INADDR_NONE )
    {
      /* We've interpreted, and stored, a valid IPv4 address; this
       * isn't, strictly, a node name; it is valid, in this context,
       * but association with a canonical name is disallowed.
       */
      if( (ai_data.ai.ai_flags & AI_CANONNAME) == AI_CANONNAME )
	return EAI_BADFLAGS;

      /* When the IPv4 address form is accepted, we may immediately
       * reset the return status, since the requirement for at least
       * one nodename, or servname, to be non-NULL is satisfied.
       */
      status = 0;
    }
    else if( (ai_data.ai.ai_flags & AI_NUMERICHOST) == AI_NUMERICHOST )
    {
      /* No numeric IPv4 address was specified, but with this flag
       * one becomes a mandatory requirement; bail out.
       */
      return EAI_NONAME;
    }
    else
    { /* The specified nodename could not be interpreted as an IPv4
       * address, (but was not required to be); resolve it as a host
       * name, to obtain the corresponding IPv4 address.
       */
      struct hostent *hi = gethostbyname( nodename );
      if( hi != NULL )
      {
	/* Name resolution was successful; update the template data
	 * to reflect the size of an IPv4 socket data structure, and
	 * store the resolved IPv4 address.
	 *
	 * FIXME: is it necessary to adapt this, to process a list
	 * comprising more than one resolved IPv4 address?
	 */
	ai_data.ai.ai_addrlen = sizeof( struct sockaddr );
	ai_data.sa.sin_addr = **(struct in_addr **)(hi->h_addr_list);

	/* Also, if requested, capture the resolved canonical host
	 * name, but fail if there is insufficient memory.
	 */
	if( ((ai_data.ai.ai_flags & AI_CANONNAME) == AI_CANONNAME)
	&&  ((ai_data.ai.ai_canonname = strdup( hi->h_name )) == NULL) )
	  return EAI_MEMORY;

	/* On successfully updating the template, to reflect all
	 * data associated with the resolved nodename, reset the
	 * exit status to indicate success.
	 */
	status = 0;
      }
    }
  }
  else if( status == 0 )
  { /* The nodename argument was effectively NULL, but we are able
     * to proceed due to prior interpretation of a non-NULL servname
     * argument; assign default addresses...
     */
    if( ((ai_data.ai.ai_flags & AI_PASSIVE) == AI_PASSIVE) )
    { /* ...corresponding to the wild-card address for any passive
       * connection...
       */
      ai_data.sa.sin_addr.s_addr = INADDR_ANY;
    }
    else
    { /* ...or the loopback address, for non-passive connections.
       */
      ai_data.sa.sin_addr.s_addr = INADDR_LOOPBACK;
    }
  }

  /* Provided the return status code has been reset to zero, from
   * its initial EAI_NONAME value...
   */
  if( status == 0 )
  { /* ...we should now have a suitably initialized template, from
     * which we may construct the linked list of matching addrinfo
     * structures to be returned.  Each list entry is separately
     * allocated, using the local pai pointer to track insertion
     * point, selecting protocol and socket type pairings from the
     * indexed list, working from TCP (at index two), back towards
     * the wild-card protocol match (at index zero), and passing
     * through UDP (at index one), until we reach the protocol as
     * specified in the template; note that this will generate a
     * single entry of TCP, or UDP, when either is specified as an
     * explicit match requirement, but will generate three entries,
     * matching each of TCP, UDP, and ANY, when a wild-card match
     * is specified.
     */
    struct ai *pai = NULL;
    int ref = (ai_data.ai.ai_protocol == IPPROTO_UDP) ? 1 : 2;
    struct { int protocol; int socktype; } map[] =
    { { __WSPIAPI_IPPROTO_ANY, __WSPIAPI_SOCK_ANY },
      { IPPROTO_UDP, SOCK_DGRAM }, { IPPROTO_TCP, SOCK_STREAM }
    };
    do { if( pai == NULL )
	 { /* When pai is in its initial NULL state, the entry to
	    * be generated may be assigned directly to the head of
	    * the return list...
	    */
	   pai = ((struct ai *)(malloc( sizeof ai_data )));
	   *res = (struct addrinfo *)(pai);
	 }
	 /* ...otherwise we link it as ai_next successor to the
	  * previously allocated entry.
	  */
	 else
	 { pai->ai.ai_next = (struct addrinfo *)(malloc( sizeof ai_data ));
	   pai = (struct ai *)(pai->ai.ai_next);
	 }
	 if( pai == NULL )
	 { /* When pai remains in, or reverts to, the NULL state,
	    * then allocation of memory for the list entry failed;
	    * clean up any partially allocated return data, before
	    * bailing out.
	    */
	   *res = __wspiapi_addrinfo_cleanup( *res );
	   return EAI_MEMORY;
	 }
	 /* We got a successfully allocated block of memory for a
	  * list entry; populate it by copying the template...
	  */
	 *pai = ai_data;

	 /* ...then fix up the embedded sockaddr data reference,
	  * its protocol, and the socket type indicators.
	  */
	 pai->ai.ai_addr = (struct sockaddr *)(&(pai->sa.sin_family));
	 pai->ai.ai_protocol = map[ref].protocol;
	 pai->ai.ai_socktype = map[ref].socktype;

	 /* The template may initially include a canonical name
	  * reference; this is propagated to the first list entry
	  * only, so clear it prior to generating any more.
	  */
	 ai_data.ai.ai_canonname = NULL;

	 /* Go round again, until the addrinfo list has become
	  * fully populated.
	  */
       } while( map[ref--].protocol != ai_data.ai.ai_protocol );
  }
  /* Ultimately, return either the initially assumed EAI_NONAME
   * failure status, or success.
   */
  return status;
}

/* __wspiapi_getaddrinfo(): a thin wrapper around the preceding
 * in-line implementation; invoked by WspiapiGetAddrInfo() if, and
 * only if, no getaddrinfo() implementation is identifiable within
 * the host system's WS2_32.DLL, or WSHIP6.DLL
 */
WSAAPI int __wspiapi_getaddrinfo (
  const char *__restrict__ nodenam, const char *__restrict__ servnam,
  const struct addrinfo *__restrict__ hints, struct addrinfo **__restrict__ res
)
{ /* First ensure that a non-NULL reference pointer is provided, to
   * pass the addrinfo result back to the caller, then delegate the
   * call to the in-line handler, capturing its exit status, which
   * is then propagated through the WSAGetLastError() API.
   */
  if( res == NULL ) return __wspiapi_syserrout( EINVAL );
  { int status = __wspiapi_getaddrinfo_internal( nodenam, servnam, hints, res );
    return (status == 0) ? 0 : __wspiapi_errout( status );
  }
}

/* WspiapiGetAddrInfo(): getaddrinfo() function redirector, as
 * prescribed by Microsoft's on-line documentation.
 */
static __inline__
WSAAPI int WspiapiGetAddrInfo
( const char *node, const char *service, const struct addrinfo *hints,
  struct addrinfo **response_list
)
{ /* Initialized to NULL, the redirector hook will be updated on
   * first call, within the containing translation unit.  This call
   * will determine if it can be directed to an actual getaddrinfo()
   * function implementation, provided by the system's WS2_32.DLL or
   * WSHIP6.DLL libraries, initializing redirection for this first,
   * and any subsequent call, to delegate to any such existing
   * implementation, if available...
   */
  typedef WSAAPI int (*call)
    ( const char *, const char *, const struct addrinfo *, struct addrinfo **
    );
  static call redirector_hook = NULL;
  if(  (redirector_hook == NULL)
  &&  ((redirector_hook = __wspiapi_call(__WSPIAPI_GETADDRINFO__)) == NULL)  )

    /* ...or otherwise, to substitute the above fallback.
     */
    redirector_hook = __wspiapi_getaddrinfo;

  /* Ultimately, every call is redirected to whichever handler has
   * been selected by the preceding initialization.
   */
  return redirector_hook( node, service, hints, response_list );
}

/* getaddrinfo(): a static inline override for any external
 * implementation of getaddrinfo(); ensures that corresponding
 * function calls, within the translation unit file scope, are
 * delegated to the fallback WspiapiGetAddrInfo() redirector.
 */
static __inline__ __attribute__((__always_inline__))
WSAAPI int getaddrinfo( const char *node, const char *service,
  const struct addrinfo *hints, struct addrinfo **response_list )
{ return WspiapiGetAddrInfo( node, service, hints, response_list ); }

/* WspiapiFreeAddrInfo(): freeaddrinfo() function redirector, as
 * prescribed by Microsoft's on-line documentation.
 */
static __inline__
WSAAPI void WspiapiFreeAddrInfo( struct addrinfo *ai_chain )
{
  /* Initialized to NULL, the redirector hook will be updated on
   * first call, within the containing translation unit.  This call
   * will determine if it can be directed to an actual freeaddrinfo()
   * function implementation, provided by the system's WS2_32.DLL or
   * WSHIP6.DLL libraries, initializing redirection for this first,
   * and any subsequent call, to delegate to any such existing
   * implementation, if available...
   */
  typedef WSAAPI void (*call)(struct addrinfo *);
  static call redirector_hook = NULL;
  if(  (redirector_hook == NULL)
  &&  ((redirector_hook = __wspiapi_call(__WSPIAPI_FREEADDRINFO__)) == NULL)  )

    /* ...or otherwise, to substitute the above fallback.
     */
    redirector_hook = __wspiapi_freeaddrinfo;

  /* Ultimately, every call is redirected to whichever handler has
   * been selected by the preceding initialization.
   */
  redirector_hook( ai_chain );
}

/* freeaddrinfo(): a static inline override for any external
 * implementation of freeaddrinfo(); ensures that corresponding
 * function calls, within the translation unit file scope, are
 * delegated to the fallback WspiapiFreeAddrInfo() redirector.
 */
static __inline__ __attribute__((__always_inline__))
WSAAPI void freeaddrinfo (struct addrinfo *ai_chain)
{ WspiapiFreeAddrInfo (ai_chain); }

/* __wspiapi_getservname(): helper to retrieve the standard name,
 * if any, for the service nominally associated with any port which
 * is specified by its number in network byte order.
 */
static __inline__ __attribute__((__always_inline__))
char *__wspiapi_getservname( u_short port, int flags, char *service,
  socklen_t servlen, socklen_t *retlen
)
{ /* This look-up is suppressed, if NI_NUMERICSERV is included
   * within the specified flags...
   */
  if( (flags & NI_NUMERICSERV) == 0 )
  {
    /* ...but, when allowed to proceed, we use the legacy windows
     * getservbyport() function to perform the look-up...
     */
    struct servent *service_info;
    service_info = getservbyport( port, (flags & NI_DGRAM) ? "udp" : NULL );

    /* ...and then, provided a valid service name has been found,
     * we transcribe it into the supplied buffer, for return.
     */
    if( (service_info != NULL) && (service_info->s_name != NULL) )
    { *retlen = snprintf( service, servlen, "%s", service_info->s_name );
      return service;
    }
  }
  /* If, for whatever reason, no service name is to be returned,
   * returning NULL will signal this.
   */
  return NULL;
}

/* __wspiapi_getnodename(): helper to perform a reverse DNS look-up,
 * to retrieve, and nominally return, the fully qualified domain name
 * which is associated with a specified IPv4 address.
 */
static __inline__ __attribute__((__always_inline__))
char *__wspiapi_getnodename( struct in_addr *addr, int flags,
  char *nodename, socklen_t nodelen, socklen_t *retlen
)
{ /* This look-up is suppressed, when NI_NUMERICHOST is included
   * within the specified flags...
   */
  if( (flags & NI_NUMERICHOST) == 0 )
  {
    /* ...but, when allowed to proceed, we use the legacy windows
     * gethostbyaddr() function to perform the look-up...
     */
    struct hostent *node_info = gethostbyaddr(
	(const char *)(addr), sizeof( struct in_addr ), AF_INET
      );

    /* ...and then, provided a valid host name has been found,
     * we transcribe it into the supplied buffer, for return.
     */
    if( (node_info != NULL) && (node_info->h_name != NULL) )
    { *retlen = snprintf( nodename, nodelen, "%s", node_info->h_name );

      /* A special case arises, when NI_NOFQDN is included
       * within the specified flags.
       */
      if( (flags & NI_NOFQDN) != 0 )
      {
	/* In this case, we must check for any period, as
	 * punctuation separating the host name itself from
	 * its domain name...
	 */
	char *brk = strchr( nodename, '.' );
	if( brk != NULL )
	{
	  /* ...and, when such punctuation is present, we
	   * truncate the return string at the first period,
	   * so leaving only the bare host name for return.
	   */
	  *brk = '\0';
	  *retlen = brk - nodename;
	}
      }
      /* Truncated, or otherwise, we may now return the host
       * name, as retrieved.
       */
      return nodename;
    }
  }
  /* If, for whatever reason, no host name is to be returned,
   * returning NULL will signal this.
   */
  return NULL;
}

/* __wspiapi_getnameinfo_internal(): core implementation for, and called
 * exclusively by, __wspiapi_getnameinfo(); returns zero on success, or
 * an EAI error code, (but without updating WSAGetLastError() status),
 * on failure.
 */
static __inline__
__attribute__((__always_inline__))
int __wspiapi_getnameinfo_internal
( const struct sockaddr *__restrict__ sa, socklen_t len,
  char *__restrict__ node, socklen_t nodelen, char *__restrict__ service,
  socklen_t servlen, int flags
)
{ /* Either the service name, or the node name look-up may be omitted,
   * but at least one MUST be performed; if neither is performed, as we
   * may establish later, then we should fail with status EAI_NONAME.
   */
  int status = EAI_NONAME;

  /* If support for IPv6 is available, then the system DLL will provide
   * its own getnameinfo() implementation, and this fallback handler is
   * not invoked; thus, we need only support the IPv4 address family.
   */
  if( sa->sa_family != AF_INET ) return EAI_FAMILY;

  /* The "service" argument may be NULL, or its associated "servlen"
   * may be zero; a service name look-up is performed, only if neither
   * of these excluding conditions is satisfied.
   */
  if( (service != NULL) && (servlen > 0) )
  {
    /* When look-up is NOT excluded, we delegate it to our internal
     * __wspiapi_getservname() helper, capturing the result into an
     * intermediate buffer of the same length as has been declared
     * for the returned result...
     */
    char servname[servlen];
    u_short port = ((const struct sockaddr_in *)(sa))->sin_port;
    if( __wspiapi_getservname( port, flags, servname, servlen, &len ) == NULL )

      /* ...while noting that, if the look-up was unsuccessful, (or it
       * was suppressed, because NI_NUMERICSERV was included within the
       * specified flags), we substitute a string representation of the
       * service port number.
       */
      len = snprintf( servname, servlen, "%u", ntohs( port ) );

    /* Provided the declared buffer length is sufficient to accommodate
     * the result of the look-up, we copy that result from intermediate
     * storage to the designated return buffer...
     */
    if( servlen > len ) strcpy( service, servname );

    /* ...but if it doesn't fit, we bail out, reporting overflow.
     */
    else return EAI_OVERFLOW;

    /* We HAVE now performed a successful service name look-up, so we
     * may clear our initial EAI_NONAME presumption.
     */
    status = 0;
  }

  /* The "node" argument may be NULL, or its associated "nodelen" may
   * be zero; a node name look-up is performed, only if neither of
   * these excluding conditions is satisfied.
   */
  if( (node != NULL) && (nodelen > 0) )
  {
    /* Perform the node name look-up, based on the IPv4 address as
     * specified in the "sa" structure argument...
     */
    char nodename[nodelen];
    struct in_addr addr = ((const struct sockaddr_in *)(sa))->sin_addr;
    if( __wspiapi_getnodename( &addr, flags, nodename, nodelen, &len ) == NULL )
    {
      /* ...but on look-up failure, and if not required to succeed,
       * (by specification of the NI_NAMEREQD flag)...
       */
      if( (flags & NI_NAMEREQD) != 0 ) return  EAI_NONAME;

      /* ...substitute the textual representation of the address.
       */
      len = snprintf( nodename, nodelen, "%s", inet_ntoa( addr ) );
    }
    /* Whether the name look-up was successful, or the address was
     * substituted, if the return "node" buffer size is sufficient,
     * copy the look-up result into place...
     */
    if( nodelen > len ) strcpy( node, nodename );

    /* ...otherwise, bail out.
     */
    else return EAI_OVERFLOW;

    /* If we get this far, the function has completed successfully.
     */
    return 0;
  }
  /* If we didn't perform "node" name look-up, then the return state
   * must be either the result of successful "service" resolution, or
   * the initially assumed failure state.
   */
  return status;
}

/* __wspiapi_getnameinfo(): a thin wrapper around the preceding
 * in-line implementation; invoked by WspiapiGetNameInfo() if, and
 * only if, no getnameinfo() implementation is identifiable within
 * the host system's WS2_32.DLL, or WSHIP6.DLL
 */
static __inline__
WSAAPI int __wspiapi_getnameinfo
( const struct sockaddr *__restrict__ sa, socklen_t len,
  char *__restrict__ node, socklen_t nodelen, char *__restrict__ service,
  socklen_t servlen, int flags
)
{ /* The "sa" argument MUST NOT be NULL, and its declared length MUST
   * match that of struct sockaddr.  RFC 3493 doesn't prescribe how we
   * should handle this case, but we choose to emulate the behaviour of
   * Microsoft's invalid parameter handler, while reporting it as an
   * EAI_SYSTEM exception.
   */
  if( (sa == NULL) || (len < (socklen_t)(sizeof( struct sockaddr ))) )
    return __wspiapi_syserrout( EINVAL );

  { /* When the "sa" argument is valid, we delegate the call to the
     * preceding in-line handler, simply capturing its return status,
     * and propagating it through the WSAGetLastError() API, as may
     * be appropriate.
     */
    int status = __wspiapi_getnameinfo_internal
    ( sa, len, node, nodelen, service, servlen, flags );
    return (status == 0) ? 0 : __wspiapi_errout( status );
  }
}

/* WspiapiGetNameInfo(): getnameinfo() function redirector, as
 * prescribed by Microsoft's on-line documentation.
 */
static __inline__
WSAAPI int WspiapiGetNameInfo
( const struct sockaddr *__restrict__ addr, socklen_t len,
  char *__restrict__ node, socklen_t nodelen, char *__restrict__ service,
  socklen_t servlen, int flags
)
{ /* Initialized to NULL, the redirector hook will be updated on
   * first call, within the containing translation unit.  This call
   * will determine if it can be directed to an actual getnameinfo()
   * function implementation, provided by the system's WS2_32.DLL or
   * WSHIP6.DLL libraries, initializing redirection for this first,
   * and any subsequent call, to delegate to any such existing
   * implementation, if available...
   */
  typedef WSAAPI int (*call)
    (const SOCKADDR *, socklen_t, char *, socklen_t, char *, socklen_t, int
    );
  static call redirector_hook = NULL;
  if(  (redirector_hook == NULL)
  &&  ((redirector_hook = __wspiapi_call(__WSPIAPI_GETNAMEINFO__)) == NULL)  )

    /* ...or otherwise, to substitute the above fallback.
     */
    redirector_hook = __wspiapi_getnameinfo;

  /* Ultimately, every call is redirected to whichever handler has
   * been selected by the preceding initialization.
   */
  return redirector_hook( addr, len, node, nodelen, service, servlen, flags );
}

/* getnameinfo(): a static inline override for any external
 * implementation of getnameinfo(); ensures that corresponding
 * function calls, within the translation unit file scope, are
 * delegated to the fallback WspiapiGetNameInfo() redirector.
 */
static __inline__ __attribute__((__always_inline__))
WSAAPI int getnameinfo( const SOCKADDR *addr, socklen_t addrlen,
  char *node, socklen_t nb_size, char *service, socklen_t sb_size, int flags
)
{ return WspiapiGetNameInfo
  (addr, addrlen, node, nb_size, service, sb_size, flags);
}

_END_C_DECLS

#endif	/* !_WSPIAPI_H: $RCSfile: wspiapi.h,v $: end of file */
