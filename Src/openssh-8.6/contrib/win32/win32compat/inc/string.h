#include "crtheaders.h"
#include STRING_H
#include "utf.h"

/* string.h overrides */
#define _stricmp strcasecmp
#define _strnicmp strncasecmp
char *w32_strerror(int);
#define strerror w32_strerror
#define strdup _strdup
#define ERROR_MSG_MAXLEN 94 /* https://msdn.microsoft.com/en-us/library/51sah927.aspx */

int __cdecl __MINGW_NOTHROW strcasecmp( const char *, const char * );
int __cdecl __MINGW_NOTHROW strncasecmp( const char *, const char *, size_t );

static char errorBuf[ERROR_MSG_MAXLEN];

char *strndup(const char*, size_t);
char * strrstr(const char *, const char *);