#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"


struct sockaddr *
addr_from_ip(char *addr, char *port)
{
    assert(addr);
    assert(port);

    int ret;
    struct addrinfo hints;
    struct addrinfo *p, *ai;
    static struct sockaddr *ss; /* return value */

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(addr, port, &hints, &ai);
    if (ret != 0) {
        perror("getaddrinfo");
        freeaddrinfo(ai);
        return NULL;
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            ss = p->ai_addr;
            break;
        }
    }

    if (p == NULL) {
        freeaddrinfo(ai);
        return NULL;
    }

    return ss;
}

int
main(void)
{
    struct sockaddr *ret = addr_from_ip("192.168.1.3", "80");

    if (ret)
        puts("Got an address");
    else
        puts("Got nothing");

    return 0;
}
