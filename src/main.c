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

    timeout.tv_sec = interval;
    timeout.tv_usec = 0;

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

        nready = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
        if (nready == -1 || errno == EINTR)
            continue;

        if (timeout.tv_sec == 0) {
            if (servinfo->is_alive) {
                printf("sending periodic update\n");
                ret = serv_broadcast(servinfo, routing_table);
                if (ret < 0)
                    serv_perror(ret);
            }

            refresh_timeouts(servinfo, routing_table);
            timeout.tv_sec = interval;
            timeout.tv_usec = 0;
        }

        if (FD_ISSET(sockfd, &rfds)) {
            ret = serv_update(servinfo, routing_table);
            if (ret < 0)
                serv_perror(ret);
            else
                packets += 1;
        }

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!fgets(inputline, MAXLEN_LINE, stdin) || ferror(stdin)) {
                fprintf(stderr, "stopping\n");
                goto cleanup;
            } else {
                cmdlen = strlen(inputline);
                inputline[cmdlen - 1] = '\0';

                tokens = tokenize(inputline);
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
                    if (from != servinfo->id) {
                        fprintf(stderr, "update: ");
                        fprintf(stderr, "invalid `src' value: %s\n", tokens[1]);
                        continue;
                    }

                    int to = validate_strtol(tokens[2]);
                    if (to == from) {
                        fprintf(stderr, "warning: updating cost to self, "
                                        "should be 0\n");
                    } else if (to < 1) {
                        fprintf(stderr, "update: ");
                        fprintf(stderr, "invalid `dest' value: %s\n", tokens[2]);
                        continue;
                    }

                    int cost = validate_strtol(tokens[3]);
                    if (!strcasecmp("inf", tokens[3])) {
                        cost = INF;
                    } else if (cost < 0) {
                        fprintf(stderr, "update: ");
                        fprintf(stderr, "invalid `cost' value: %s\n", tokens[3]);
                        continue;
                    }

                    ret = serv_send_update(servinfo, routing_table, to, cost);
                    if (ret < 0) {
                        fprintf(stderr, "update: error while sending update, ");
                        fprintf(stderr, "aborting update operation\n");
                        continue;
                    }

                    if (!table_update_cost(routing_table, to, cost))
                        fprintf(stderr, "update: arguments out of range\n");
                    else
                        printf("%s SUCCESS", inputline);
                } else if (!strcasecmp("step", cmd_name)) {
                    /* no args */
                    if (!serv_broadcast(servinfo, routing_table)) {
                        printf("sent a packet to neighbors\n");
                        printf("%s SUCCESS\n", inputline);
                    }
                } else if (!strcasecmp("packets", cmd_name)) {
                    /* no args */
                    printf("packets received: %d\n", packets);
                    packets = 0;
                    printf("%s SUCCESS\n", inputline);
                } else if (!strcasecmp("disable", cmd_name)) {
                    /* disable servid */
                    if (!tokens[1] || tokens[2]) {
                        fprintf(stderr, "usage: disable <server-id>\n");
                        continue;
                    }

                    int servid = validate_strtol(tokens[1]);
                    if (servid < 0 || servid == servinfo->id) {
                        fprintf(stderr, "disable: invalid `server-id' value\n");
                        continue;
                    }

                    if (!table_is_neighbor(routing_table, servid)) {
                        fprintf(stderr, "disable: ");
                        fprintf(stderr, "server %d not a neighbor\n", servid);
                        continue;
                    }

                    ret = serv_disable_neighbor(routing_table, servid);
                    if (ret < 0)
                        serv_perror(ret);
                    else
                        printf("%s SUCCESS\n", inputline);
                } else if (!strcasecmp("crash", cmd_name)) {
                    /* no args */
                    if (!servinfo->is_alive)
                        fprintf(stderr, "crash: already dead\n");
                    else {
                        serv_crash(servinfo);
                        printf("%s SUCCESS\n", inputline);
                    }
                } else if (!strcasecmp("display", cmd_name)) {
                    /* no args */
                    printf("%s", table_str_for_id(routing_table));
                    printf("%s SUCCESS\n", inputline);
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
