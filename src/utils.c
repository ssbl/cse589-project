#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
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

char *
get_localip(void)
{
    static char ip[INET_ADDRSTRLEN];

    const char *retptr;
    int sockfd, retval;
    socklen_t socklen;
    struct sockaddr_in *tmp, resolv_addr;
    struct sockaddr_storage sa;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return NULL;
    }

    memset(&resolv_addr, 0, sizeof(resolv_addr));
    resolv_addr.sin_family = AF_INET;
    resolv_addr.sin_port = htons(53);
    retval = inet_pton(AF_INET, RESOLV_IP, &resolv_addr.sin_addr);
    if (retval <= 0) {
        perror("inet_pton");
        return NULL;
    }

    retval = connect(sockfd, (struct sockaddr *) &resolv_addr,
                     sizeof(resolv_addr));
    if (retval < 0) {
        perror("connect");
        return NULL;
    }

    socklen = sizeof(sa);
    if (getsockname(sockfd, (struct sockaddr *) &sa, &socklen) < 0) {
        perror("getsockname");
        return NULL;
    }

    tmp = (struct sockaddr_in *) &sa;
    retptr = inet_ntop(AF_INET, &tmp->sin_addr, ip, INET_ADDRSTRLEN);
    if (!retptr) {
        perror("inet_ntop");
        return NULL;
    }

    close(sockfd);
    return ip;
}
