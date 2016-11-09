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
    int interval, sockfd, self_id, packets = 0;
    char *port;
    struct timeval timeout;
    struct table *routing_table = NULL;
    struct servinfo *servinfo = NULL;
    struct serventry *our_entry = NULL;

    char *endptr;

    if (argc != 5) {
        fprintf(stderr, USAGE_STR);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "-t") || strcmp(argv[3], "-i")) {
        fprintf(stderr, USAGE_STR);
        exit(EXIT_FAILURE);
    }

    printf("parsing topology file...\n");
    routing_table = parse_topofile(argv[2]);
    if (!routing_table) {
        fprintf(stderr, "error parsing file: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    interval = strtol(argv[4], &endptr, 10);
    if (interval < 0) {
        fprintf(stderr, "invalid interval value: %s\n", argv[4]);
        table_free(routing_table);
        exit(EXIT_FAILURE);
    }

    self_id = routing_table->id;
    printf("found matching entry with id %d\n", self_id);
    our_entry = table_lookup_server_by_id(routing_table, self_id);
    port = our_entry->port;

    sockfd = listen_socket(port);
    if (sockfd == -1) {
        table_free(routing_table);
        exit(EXIT_FAILURE);
    }

    servinfo = servinfo_init(self_id, sockfd, (time_t) interval);

    for (; ;) {
        int nready, maxfd = sockfd;
        fd_set rfds;

        int cmdlen, ret;
        char inputline[MAXLEN_LINE + 1];
        char **tokens, *cmd_name;

        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        FD_SET(STDIN_FILENO, &rfds);

        timeout.tv_sec = interval;
        timeout.tv_usec = 0;

        nready = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
        if (nready == -1 || errno == EINTR)
            continue;
        if (nready == 0 && servinfo->is_alive) { /* read, broadcast here */
            printf("sending periodic update\n");
            serv_broadcast(servinfo, routing_table);
            continue;
        }

        /* printf("waited %d seconds, got %d fds\n", interval, nready); */

        if (FD_ISSET(sockfd, &rfds)) {
            /* handle update reads here */
            ret = serv_update(servinfo, routing_table);
            if (ret == E_SYSCALL)
                fprintf(stderr, "update error: system call\n");
            else if (ret == E_UNPACK)
                fprintf(stderr, "update error: bad message format\n");
            else if (ret == E_LOOKUP)
                fprintf(stderr, "update error: lookup failed\n");
            else
                packets += 1;
        }

        /* TODO: clarify id namespace */
        /* TODO: ask about differing bidirectional costs */
        /* TODO: verify update function */
        /* TODO: check if addresses and ports in topology file are valid */
        /* TODO: create ERROR MESSAGE, SUCCESS function */
        /* TODO: print id of sender server in serv_update */
        /* TODO: create valid, non-bidirectional topology and test on that */
        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!fgets(inputline, MAXLEN_LINE, stdin) || ferror(stdin)) {
                fprintf(stderr, "stopping\n");
                goto cleanup;
            } else {
                cmdlen = strlen(inputline);
                inputline[cmdlen - 1] = '\0';

                tokens = tokenize(inputline, routing_table, servinfo);
                if (tokens == NULL || !tokens[0])
                    continue;

                cmd_name = tokens[0];
                if (!strcasecmp("update", cmd_name)) {
                    /* update src dest cost */
                    if (!tokens[1] || !tokens[2] || !tokens[3]) {
                        fprintf(stderr, "usage: update <src> <dest> <cost>\n");
                        continue;
                    }

                    int from = validate_strtol(tokens[1]);
                    if (from < 1 || from > routing_table->n) {
                        fprintf(stderr, "invalid `src' value: %s\n", tokens[1]);
                        continue;
                    }

                    int to = validate_strtol(tokens[2]);
                    if (to < 1 || to > routing_table->n) {
                        fprintf(stderr, "invalid `dest' value: %s\n", tokens[2]);
                        continue;
                    } /* TODO: check if to == from */

                    int cost = validate_strtol(tokens[3]);
                    if (!strcasecmp("inf", tokens[3])) {
                        cost = INF;
                    } else if (cost <= 0) {
                        fprintf(stderr, "invalid `cost' value: %s\n", tokens[3]);
                        continue;
                    }

                    if (!table_update_cost(routing_table, to, from, cost))
                        fprintf(stderr, "arguments out of range\n");
                } else if (!strcasecmp("step", cmd_name)) {
                    /* no args */
                    if (!serv_broadcast(servinfo, routing_table))
                        printf("sent a packet to neighbors\n");
                } else if (!strcasecmp("packets", cmd_name)) {
                    /* no args */
                    printf("packets received: %d\n", packets);
                    packets = 0;
                } else if (!strcasecmp("disable", cmd_name)) {
                    /* disable servid */
                    if (!tokens[1]) {
                        fprintf(stderr, "usage: disable <server-id>\n");
                        continue;
                    }

                    int servid = validate_strtol(tokens[1]);
                    if (servid < 0 || servid > routing_table->n) {
                        fprintf(stderr, "invalid `server-id' value\n");
                        continue;
                    }

                    if (!table_is_neighbor(routing_table, servid)) {
                        fprintf(stderr, "server %d not a neighbor\n", servid);
                        continue;
                    }

                    if (!table_update_cost(routing_table, self_id, servid, INF))
                        fprintf(stderr, "invalid servid: %s", tokens[1]);
                } else if (!strcasecmp("crash", cmd_name)) {
                    /* no args */
                    serv_crash(servinfo);
                } else if (!strcasecmp("display", cmd_name)) {
                    /* no args */
                    printf("%s", table_str_for_id(routing_table));
                } else
                    continue;
            }
        }
    }

cleanup:
    close(sockfd);
    table_free(routing_table);
    free(servinfo);
    return 0;
}
