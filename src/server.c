#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include "list.h"
#include "dvec.h"
#include "serventry.h"
#include "table.h"
#include "msg.h"
#include "utils.h"
#include "server.h"

struct servinfo *
servinfo_init(int id, int sockfd, time_t interval)
{
    struct servinfo *servinfo = NULL;;

    if ((servinfo = malloc(sizeof(*servinfo))) == NULL)
        return NULL;

    assert(id > 0);
    assert(sockfd > 0);
    assert(interval > 0);

    servinfo->id = id;
    servinfo->sockfd = sockfd;
    servinfo->interval = interval;

    return servinfo;
}

int
serv_broadcast(struct servinfo *servinfo, struct table *table)
{
    assert(servinfo);
    assert(table);
    assert(table->costs);

    int ret, servid = servinfo->id, msglen;
    unsigned char *msg;
    socklen_t addrlen;
    struct sockaddr *servaddr;

    struct listitem *ptr = table->servers->head;
    struct serventry *s_entry = NULL;

    msg = msg_pack_dvec(servid, table);
    msglen = 8 + table->n * 12;
    msg[msglen] = 0;

    while (ptr) {
        s_entry = ptr->value;

        if (s_entry->servid == servid || !s_entry->neighbor) {
            ptr = ptr->next;
            continue;
        }

        servaddr = addr_from_ip(s_entry->addr, s_entry->port);
        if (!servaddr)          /* TODO: debug this */
            continue;
        addrlen = sizeof(*servaddr);

        fprintf(stderr, "sending to server %d\n", s_entry->servid);
        ret = sendto(servinfo->sockfd, msg, msglen, 0, servaddr, addrlen);
        if (ret == -1)
            fprintf(stderr, "sendto failed for server %d", s_entry->servid);

        ptr = ptr->next;
    }

    return 0;
}

int
serv_crash(struct servinfo *servinfo)
{
    assert(servinfo);
    assert(servinfo->sockfd > 0);

    int ret;

    ret = close(servinfo->id);
    if (ret == -1)
        perror("close");

    return ret;
}

int
serv_update(struct servinfo *servinfo, struct table *table)
{
    assert(servinfo);
    assert(table);

    int cost, cost_to_sender, msglen = 8 + table->n * 12;
    int servid = servinfo->id, senderid;
    ssize_t n;
    unsigned char msg[RECVLINES + 1];
    struct dvec *recvd_dv = NULL, *our_dv = NULL;
    struct dvec_entry *recvd_dv_entry, *our_dv_entry;
    struct listitem *recvd_dvptr, *our_dvptr;

    /* todo: use a specific address, so that we don't receive it from anyone */
    n = recvfrom(servinfo->sockfd, msg, msglen, 0, NULL, NULL);
    fprintf(stderr, "received %d bytes\n", (int) n);
    if (n == -1) {
        perror("recvfrom");
        return E_SYSCALL;
    }

    recvd_dv = msg_unpack_dvec(msg, servid, table);
    dvec_print(recvd_dv);
    if (!recvd_dv || recvd_dv->from == servid)
        return E_UNPACK;

    our_dv = table_get_dvec(table, servid);
    if (!our_dv)                /* we've made a big mistake */
        return E_LOOKUP;

    senderid = recvd_dv->from;
    cost_to_sender = table_get_cost(table, servid, senderid);
    assert(cost >= 0);

    recvd_dvptr = recvd_dv->list->head;
    our_dvptr = our_dv->list->head;

    while (our_dvptr && recvd_dvptr) {
        our_dv_entry = our_dvptr->value;
        recvd_dv_entry = recvd_dvptr->value;

        cost = cost_to_sender + recvd_dv_entry->cost;
        if (our_dv_entry->cost > cost) {
            our_dv_entry->cost = cost;
            our_dv_entry->via = senderid;
        }

        our_dvptr = our_dvptr->next;
        recvd_dvptr = recvd_dvptr->next;
    }

    return 0;
}
