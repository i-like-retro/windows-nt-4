#ifndef NTCOMPAT_ERRNO_H
#define NTCOMPAT_ERRNO_H

#include <errno.h>

#ifndef ENOENT
#define ENOENT 2
#endif

#ifndef EBADF
#define EBADF 9
#endif

#ifndef ENOMEM
#define ENOMEM 12
#endif

#ifndef EACCES
#define EACCES 13
#endif

#ifndef EFAULT
#define EFAULT 14
#endif

#ifndef ERANGE
#define ERANGE 34
#endif

#ifndef EADDRINUSE
#define EADDRINUSE 100
#endif

#ifndef EAFNOSUPPORT
#define EAFNOSUPPORT 102
#endif

#ifndef ECONNABORTED
#define ECONNABORTED 106
#endif

#ifndef ECONNREFUSED
#define ECONNREFUSED 107
#endif

#ifndef ECONNRESET
#define ECONNRESET 108
#endif

#ifndef EINPROGRESS
#define EINPROGRESS 112
#endif

#ifndef ENOTCONN
#define ENOTCONN 126
#endif

#ifndef ENOTSOCK
#define ENOTSOCK 128
#endif

#ifndef ENOTSUP
#define ENOTSUP 129
#endif

#ifndef EOPNOTSUPP
#define EOPNOTSUPP 130
#endif

#ifndef EOTHER
#define EOTHER 131
#endif

#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif

#ifndef EWOULDBLOCK
#define EWOULDBLOCK 140
#endif

#endif
