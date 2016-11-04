#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>

#include "dvec.h"
#include "list.h"
#include "table.h"
#include "serventry.h"
#include "topo.h"
#include "msg.h"
#include "server.h"
#include "utils.h"
#include "main.h"

int
main(int argc, char *argv[])
{
    int interval, sockfd;
    struct table *routing_table = NULL;
    struct servinfo *servinfo = NULL;

    char *endptr;

    if (argc != 5) {
        fprintf(stderr, USAGE_STR);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-t") || strcmp(argv[3], "-i")) {
        fprintf(stderr, USAGE_STR);
        exit(EXIT_FAILURE);
    }

    routing_table = parse_topofile(argv[2]);
    if (!routing_table) {
        fprintf(stderr, "error parsing file: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    interval = strtol(argv[4], &endptr, 10);
    if (errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "invalid interval value: %s\n", argv[4]);
        table_free(routing_table);
        exit(EXIT_FAILURE);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        perror("socket");
        table_free(routing_table);
        exit(EXIT_FAILURE);
    }

    servinfo = servinfo_init(routing_table->id, sockfd, (time_t) interval);

    close(sockfd);
    table_free(routing_table);
    free(servinfo);
    return 0;
}
