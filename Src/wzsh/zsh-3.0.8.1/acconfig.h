
/***** begin user configuration section *****/

/* Define this to be the location of your password file */
#define PASSWD_FILE "/etc/passwd"

/* Define this to be the name of your NIS/YP password *
 * map (if applicable)                                */
#define PASSWD_MAP "passwd.byname"

/* Define to 1 if you want user names to be cached */
#define CACHE_USERNAMES 1

/* Define to 1 if system supports job control */
#define JOB_CONTROL 1

/* Define this if you use "suspended" instead of "stopped" */
#define USE_SUSPENDED 1
 
/* The default history buffer size in lines */
#define DEFAULT_HISTSIZE 30

/* The default editor for the fc builtin */
#define DEFAULT_FCEDIT "vi"

/* The default prefix for temporary files */
#define DEFAULT_TMPPREFIX "/tmp/zsh"


/***** end of user configuration section            *****/
/***** shouldn't have to change anything below here *****/
@TOP@

/* The global file to source absolutely first whenever zsh is run; *
 * if undefined, don't source anything                             */
#undef GLOBAL_ZSHENV

/* The global file to source whenever zsh is run; *
 * if undefined, don't source anything            */
#undef GLOBAL_ZSHRC

/* The global file to source whenever zsh is run as a login shell; *
 * if undefined, don't source anything                             */
#undef GLOBAL_ZLOGIN

/* The global file to source whenever zsh is run as a login shell, *
 * before zshrc is read; if undefined, don't source anything       */
#undef GLOBAL_ZPROFILE

/* The global file to source whenever zsh was run as a login shell.  *
 * This is sourced right before exiting.  If undefined, don't source *
 * anything                                                          */
#undef GLOBAL_ZLOGOUT

/* Define to 1 if compiler could initialise a union */
#undef HAVE_UNION_INIT

/* Define to 1 if compiler incorrectly cast signed to unsigned */
#undef BROKEN_SIGNED_TO_UNSIGNED_CASTING

/* Define if your system defines TIOCGWINSZ in sys/ioctl.h.  */
#undef GWINSZ_IN_SYS_IOCTL

/* Define to 1 if you have NIS */
#undef HAVE_NIS

/* Define to 1 if you have NISPLUS */
#undef HAVE_NIS_PLUS

/* Define to 1 if you have RFS superroot directory. */
#undef HAVE_SUPERROOT

/* Define to the path of the /dev/fd filesystem */
#undef PATH_DEV_FD

/* Define if sys/time.h and sys/select.h cannot be both included */
#undef TIME_H_SELECT_H_CONFLICTS

/* Define if your system's struct utmp has a member named ut_host.  */
#undef HAVE_UT_HOST

/* Define if you have the <utmpx.h> header file.  */
#undef HAVE_UTMPX_H

/* Define to be the machine type (microprocessor class or machine model) */
#undef MACHTYPE

/* Define to be the name of the operating system */
#undef OSTYPE

/* Define to 1 if ANSI function prototypes are usable.  */
#undef PROTOTYPES

/* Define to be location of utmp file.  This value is only used if UTMP_FILE, *
 * UTMPX_FILE, or _PATH_UTMP are not defined in an include file.              */
#undef UTMP_FILE_CONFIG

/* Define to be a string corresponding the vendor of the machine */
#undef VENDOR

/* Define if your system defines `struct winsize' in sys/ptem.h.  */
#undef WINSIZE_IN_PTEM

/* Define  to be location of wtmp file.  This value is only use if WTMP_FILE, *
 * WTMPX_FILE, or _PATH_WTMP are not defined in an include file.              */
#undef WTMP_FILE_CONFIG

/* Define to 1 if you want to debug zsh */
#undef DEBUG

/* Define to 1 if you want to use zsh's own memory allocation routines */
#undef ZSH_MEM

/* Define to 1 if you want to debug zsh memory allocation routines */
#undef ZSH_MEM_DEBUG

/* Define to 1 if you want to turn on warnings of memory allocation errors */
#undef ZSH_MEM_WARNING

/* Define to 1 if you want to turn on memory checking for free() */
#undef ZSH_SECURE_FREE

/* Define to 1 if you want to get debugging information on internal *
 * hash tables.  This turns on the `hashinfo' builtin.              */
#undef ZSH_HASH_DEBUG

/* Define if you want locale features.  By default this is defined. */
#undef CONFIG_LOCALE

/* Define to 1 if your termcap library has the ospeed variable */
#undef HAVE_OSPEED
/* Define to 1 if you have ospeed, but it is not defined in termcap.h */
#undef MUST_DEFINE_OSPEED

/* Define to 1 if tgetent() accepts NULL as a buffer */
#undef TGETENT_ACCEPTS_NULL

/* Define to 1 if you use POSIX style signal handling */
#undef POSIX_SIGNALS

/* Define to 1 if you use BSD style signal handling (and can block signals) */
#undef BSD_SIGNALS

/* Define to 1 if you use SYS style signal handling (and can block signals) */
#undef SYSV_SIGNALS

/* Define to 1 if you have no signal blocking at all (bummer) */
#undef NO_SIGNAL_BLOCKING

/* Define to `unsigned int' if <sys/types.h> or <signal.h> doesn't define */
#undef sigset_t

/* Define to 1 if struct timezone is defined by a system header */
#undef HAVE_STRUCT_TIMEZONE

/* Define to 1 if there is a prototype defined for brk() on your system */
#undef HAVE_BRK_PROTO

/* Define to 1 if there is a prototype defined for sbrk() on your system */
#undef HAVE_SBRK_PROTO

/* Define to 1 if there is a prototype defined for ioctl() on your system */
#undef HAVE_IOCTL_PROTO

/* Define to 1 if there is a prototype defined for mknod() on your system */
#undef HAVE_MKNOD_PROTO

/* Define to 1 if system has working FIFO's */
#undef HAVE_FIFOS

/* Define to 1 if struct rlimit uses quad_t */
#undef RLIM_T_IS_QUAD_T

/* Define to 1 if struct rlimit uses long long */
#undef RLIM_T_IS_LONG_LONG

/* Define to 1 if rlimit uses unsigned */
#undef RLIM_T_IS_UNSIGNED

/* Define to the type used in struct rlimit */
#undef rlim_t

/* Define to 1 if /bin/sh does not interpret \ escape sequences */
#undef SH_USE_BSD_ECHO

/* Define to `unsigned long' if <sys/types.h> doesn't define. */
#undef ino_t

/*
 * Definitions used when a long is less than eight byte, to try to
 * provide some support for eight byte operations.
 *
 * Note that ZSH_64_BIT_TYPE, OFF_T_IS_64_BIT, INO_T_IS_64_BIT do *not* get
 * defined if long is already 64 bits, since in that case no special handling
 * is required.
 */

/* Define to a 64 bit integer type if there is one, but long is shorter */
#undef ZSH_64_BIT_TYPE

/* Define to an unsigned variant of ZSH_64_BIT_TYPE if that is defined */
#undef ZSH_64_BIT_UTYPE

/* Define to 1 if off_t is 64 bit (for large file support) */
#undef OFF_T_IS_64_BIT

/* Define to 1 if ino_t is 64 bit (for large file support) */
#undef INO_T_IS_64_BIT
