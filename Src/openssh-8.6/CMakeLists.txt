cmake_minimum_required(VERSION 3.0)
project(OpenSSH)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../_ntcompat")

add_definitions(
    -DWIN32
    -D_WIN32_WINNT=0x400
    -DWINVER=0x400
    -D_FILE_OFFSET_BITS=64
    -D_LIB
    -D_MBCS
    )

include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/includes"
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/win32/win32compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/win32/win32compat/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/contrib/win32/openssh"
    "${CMAKE_CURRENT_SOURCE_DIR}/libkrb"
    "${CMAKE_CURRENT_SOURCE_DIR}/libkrb/libKrb5"
    "${CMAKE_CURRENT_SOURCE_DIR}/../openssl-1.1.1m/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../_build/zlib"
    "${CMAKE_CURRENT_SOURCE_DIR}/../zlib-1.2.11"
    )

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/contrib/win32/win32compat/inc/crtheaders.h")
    set(src "${CMAKE_CURRENT_SOURCE_DIR}/../../Toolchain/TDM-GCC/include")
    set(f "")
    set(f "${f}#define STDIO_H \"${src}/stdio.h\"\n")
    set(f "${f}#define STRING_H \"${src}/string.h\"\n")
    set(f "${f}#define SYS_TYPES_H \"${src}/sys/types.h\"\n")
    set(f "${f}#define CTYPE_H \"${src}/ctype.h\"\n")
    set(f "${f}#define STDLIB_H \"${src}/stdlib.h\"\n")
    set(f "${f}#define SYS_STAT_H \"${src}/sys/stat.h\"\n")
    set(f "${f}#define SYS_TIME_H \"${src}/sys/time.h\"\n")
    set(f "${f}#define FCNTL_H \"${src}/fcntl.h\"\n")
    set(f "${f}#define TIME_H \"${src}/time.h\"\n")
    file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/contrib/win32/win32compat/inc/crtheaders.h" "${f}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-ident -fno-stack-protector -fno-unwind-tables -fno-asynchronous-unwind-tables -march=i586")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--build-id=none -Wl,-s")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id=none -Wl,-s")

add_library(libssh STATIC
    addr.c
    addrmatch.c
    atomicio.c
    authfd.c
    authfile.c
    bitmap.c
    canohost.c
    chacha.c
    channels.c
    cipher-aes.c
    cipher-aesctr.c
    cipher-chachapoly.c
    cipher-ctr.c
    cipher.c
    cleanup.c
    compat.c
    dh.c
    dispatch.c
    dns.c
    ed25519.c
    entropy.c
    fatal.c
    fe25519.c
    ge25519.c
    #gss-genr.c
    hash.c
    hmac.c
    hostfile.c
    kex.c
    kexc25519.c
    kexdh.c
    kexecdh.c
    kexgex.c
    kexgexc.c
    kexgen.c
    kexsntrup761x25519.c
    krl.c
    log.c
    mac.c
    match.c
    misc.c
    moduli.c
    monitor_fdpass.c
    msg.c
    nchan.c
    packet.c
    poly1305.c
    progressmeter.c
    readpass.c
    rijndael.c
    sc25519.c
    smult_curve25519_ref.c
    sntrup761.c
    ssh-dss.c
    ssh-ecdsa.c
    ssh-ed25519.c
    ssh-pkcs11.c
    ssh-rsa.c
    sshbuf-getput-basic.c
    sshbuf-getput-crypto.c
    sshbuf-misc.c
    sshbuf.c
    ssherr.c
    sshkey.c
    ssh_api.c
    umac.c
    verify.c
    xmalloc.c
    platform-misc.c
    platform-pledge.c
    platform-tracing.c
    platform.c
    sandbox-pledge.c
    digest-openssl.c
    kexgexs.c
    ssh-ecdsa-sk.c
    ssh-ed25519-sk.c
    ssh-sk.c
    sshbuf-io.c
    cipher-chachapoly-libcrypto.c
    readconf.c
    mux.c
    openbsd-compat/arc4random.c
    openbsd-compat/base64.c
    openbsd-compat/basename.c
    openbsd-compat/bcrypt_pbkdf.c
    openbsd-compat/bindresvport.c
    openbsd-compat/blowfish.c
    openbsd-compat/bsd-asprintf.c
    #openbsd-compat/bsd-closefrom.c
    openbsd-compat/bsd-cygwin_util.c
    openbsd-compat/bsd-getline.c
    openbsd-compat/bsd-getpagesize.c
    #openbsd-compat/bsd-getpeereid.c
    openbsd-compat/bsd-misc.c
    openbsd-compat/bsd-nextstep.c
    openbsd-compat/bsd-openpty.c
    openbsd-compat/bsd-poll.c
    openbsd-compat/bsd-setres_id.c
    openbsd-compat/bsd-snprintf.c
    openbsd-compat/bsd-statvfs.c
    openbsd-compat/bsd-waitpid.c
    openbsd-compat/daemon.c
    openbsd-compat/dirname.c
    openbsd-compat/explicit_bzero.c
    openbsd-compat/fake-rfc2553.c
    openbsd-compat/fmt_scaled.c
    openbsd-compat/getcwd.c
    openbsd-compat/getgrouplist.c
    openbsd-compat/getopt_long.c
    openbsd-compat/getrrsetbyname-ldns.c
    openbsd-compat/inet_aton.c
    openbsd-compat/inet_ntoa.c
    openbsd-compat/inet_ntop.c
    openbsd-compat/kludge-fd_set.c
    #openbsd-compat/libressl-api-compat.c
    openbsd-compat/md5.c
    openbsd-compat/memmem.c
    openbsd-compat/mktemp.c
    openbsd-compat/openssl-compat.c
    openbsd-compat/port-irix.c
    openbsd-compat/port-linux.c
    openbsd-compat/port-solaris.c
    openbsd-compat/port-net.c
    openbsd-compat/port-uw.c
    openbsd-compat/readpassphrase.c
    openbsd-compat/reallocarray.c
    openbsd-compat/recallocarray.c
    openbsd-compat/rresvport.c
    openbsd-compat/setenv.c
    #openbsd-compat/setproctitle.c
    openbsd-compat/sha1.c
    openbsd-compat/sha2.c
    openbsd-compat/strmode.c
    openbsd-compat/strptime.c
    openbsd-compat/strsep.c
    openbsd-compat/strtoll.c
    openbsd-compat/strtonum.c
    openbsd-compat/strtoul.c
    openbsd-compat/strtoull.c
    openbsd-compat/timingsafe_bcmp.c
    openbsd-compat/vis.c
    openbsd-compat/xcrypt.c
    openbsd-compat/glob.c
    openbsd-compat/strcasestr.c
    openbsd-compat/strndup.c
    contrib/win32/win32compat/pwd.c
    contrib/win32/win32compat/fileio.c
    contrib/win32/win32compat/termio.c
    contrib/win32/win32compat/w32fd.c
    contrib/win32/win32compat/w32log.c
    #contrib/win32/win32compat/gss-sspi.c
    contrib/win32/win32compat/ttymodes_windows.c
    contrib/win32/win32compat/w32-sshfileperm.c
    contrib/win32/win32compat/misc.c
    contrib/win32/win32compat/spawn-ext.c
    contrib/win32/win32compat/socketio.c
    contrib/win32/win32compat/utf.c
    contrib/win32/win32compat/win32-utf8.c
    contrib/win32/win32compat/win32_sshtty.c
    contrib/win32/win32compat/signal.c
    contrib/win32/win32compat/signal_wait.c
    contrib/win32/win32compat/spawn.c
    contrib/win32/win32compat/win32_usertoken_utils.c
    contrib/win32/openssh/sshTelemetry.c
    )

target_link_libraries(libssh
    "${CMAKE_CURRENT_SOURCE_DIR}/../_ntcompat/libntcompat.dll.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../openssl-1.1.1m/libcrypto.dll.a"
    "${CMAKE_CURRENT_SOURCE_DIR}/../_build/zlib/libzlib.dll.a"
    ws2_32
    shlwapi
    )

add_executable(ssh
    ssh.c
    sshconnect.c
    clientloop.c
    )

target_link_libraries(ssh libssh)
