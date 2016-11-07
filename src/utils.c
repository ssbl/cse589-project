#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
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

    int ret, portnum;
    static struct sockaddr_in ss;
    static struct sockaddr *sa_addr; /* return value */

    memset(&ss, 0, sizeof ss);
    ss.sin_family = AF_INET;

    ret = inet_pton(AF_INET, addr, &(ss.sin_addr));
    if (!ret) {
        fprintf(stderr, "addr_from_ip: address not in correct format\n");
        return NULL;
    }

    portnum = validate_strtol(port);
    if (portnum < 0) {
        fprintf(stderr, "addr_from_ip: invalid port\n");
        return NULL;
    }
    ss.sin_port = htons(portnum);

    sa_addr = (struct sockaddr *) &ss;
    return sa_addr;
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

int
listen_socket(char *port)
{
    assert(port);

    int sockfd, p;
    struct sockaddr_in sa;

    char *endptr;

    p = strtol(port, &endptr, 10);
    if (errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "invalid port number\n");
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == 1) {
        perror("socket");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sin_port = htons(p);
    sa.sin_family = AF_INET;
    if (bind(sockfd, (struct sockaddr *) &sa, sizeof(sa)) == -1) {
        perror("bind");
        close(sockfd);
        return -1;
    }

    printf("listening on port %d\n", p);
    return sockfd;
}

int
max(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

int
validate_strtol(char *s)
{
    char *endptr;
    int l = strtol(s, &endptr, 10);

    if (errno == ERANGE || *endptr != '\0')
        return -1;
    return l;
}

char **
tokenize(char *inputline)
{
    assert(inputline);
    assert(table);
    assert(servinfo);

    int tok;
    static char *tokens[4];     /* return value */
    char *str, *token, *delim, *saveptr;

    for (tok = 0, str = inputline, delim = " \n"; ; tok++, str = NULL) {
        token = strtok_r(str, delim, &saveptr);
        if (!token)
            break;
        tokens[tok] = token;
    }

    if (!tok)
        return NULL;
    if (tok == 5) {
        fprintf(stderr, "too many arguments in command\n");
        return NULL;
    }

    return tokens;
}
