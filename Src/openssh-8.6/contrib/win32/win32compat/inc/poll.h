#pragma once

#include "sys\types.h"
#include "sys\socket.h"

#define POLLRDNORM  0x0100
#define POLLRDBAND  0x0200
#define POLLIN      (POLLRDNORM | POLLRDBAND)

#define POLLWRNORM  0x0010
#define POLLOUT     (POLLWRNORM)

#define POLLERR     0x0001

/* created to #def out decarations in open-bsd.h (that are defined in winsock2.h) */

int poll(struct pollfd *, nfds_t, int);