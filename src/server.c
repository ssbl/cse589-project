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
    servinfo->is_alive = 1;
    servinfo->interval = interval;

    return servinfo;
}

void
reset_timeout(struct table *table, int servid)
{
    assert(table);

    struct serventry *s_entry = table_lookup_server_by_id(table, servid);
    assert(s_entry);

    s_entry->lastrecvd = 0;
}

void
refresh_timeouts(struct servinfo *servinfo, struct table *table)
{
    assert(servinfo);

    int interval = (int) servinfo->interval;
    int neighbor_id, our_id = servinfo->id;
    struct serventry *s_entry = NULL;
    struct listitem *ptr = table->servers->head;

    while (ptr) {
        s_entry = ptr->value;

        if (!s_entry->neighbor) {
            ptr = ptr->next;
            continue;
        }

        neighbor_id = s_entry->servid;
        if (neighbor_id == our_id)
            ;                   /* skip our entry */
        else if (s_entry->lastrecvd >= 3 * interval)
            table_update_cost(table, neighbor_id, INF);
        else
            s_entry->lastrecvd += interval;

        ptr = ptr->next;
    }
}

int
update_from_id(struct servinfo *servinfo, struct table *table,
               unsigned char *msg)
{
    assert(servinfo);
    assert(table);
    assert(msg);

    int from, cost;

    if (!msg_unpack_update(msg, &from, &cost))
        return E_UNPACK;

    printf("cost %d from %d\n", cost, from);
    if (from == servinfo->id || !table_update_cost(table, from, cost))
        return E_LOOKUP;

    printf("got an update from %d\n", from);
    return 0;
}

int
serv_send_update(struct servinfo *servinfo, struct table *table,
            int neighbor_id, int cost)
{
    assert(servinfo);
    assert(table);

    int ret;
    socklen_t addrlen;
    unsigned char msg[8];
    struct sockaddr *servaddr = NULL;
    struct serventry *s_entry = NULL;

    if (!msg_pack_update(msg, servinfo->id, cost))
        return E_PACK;
    msg[8] = 0;

    s_entry = table_lookup_server_by_id(table, neighbor_id);
    if (!s_entry)
        return E_LOOKUP;

    servaddr = addr_from_ip(s_entry->addr, s_entry->port);
    printf("sending update to %d\n", neighbor_id);
    if (!servaddr)
        return E_BADADDR;

    addrlen = sizeof(*servaddr);
    ret = sendto(servinfo->sockfd, msg, 8, 0, servaddr, addrlen);
    if (ret == -1) {
        perror("sendto");
        return E_SYSCALL;
    }

    return 0;
}

int
serv_disable_neighbor(struct table *table, int neighbor_id)
{
    assert(table);

    struct serventry *s_entry = NULL;

    if (!(s_entry = table_lookup_server_by_id(table, neighbor_id)))
        return E_LOOKUP;
    if (!table_update_cost(table, neighbor_id, INF))
        return E_LOOKUP;

    s_entry->neighbor = 0;

    return 0;
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
    if (!msg)
        return E_PACK;

    msglen = 8 + table->n * 12;
    msg[msglen] = 0;

    while (ptr) {
        s_entry = ptr->value;

        if (s_entry->servid == servid || !s_entry->neighbor) {
            ptr = ptr->next;
            continue;
        }

        servaddr = addr_from_ip(s_entry->addr, s_entry->port);
        if (!servaddr)
            return E_BADADDR;

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

    servinfo->is_alive = 0;

    return 0;
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

    n = recvfrom(servinfo->sockfd, msg, msglen, 0, NULL, NULL);
    fprintf(stderr, "received %d bytes\n", (int) n);
    if (n == -1) {
        perror("recvfrom");
        return E_SYSCALL;
    } else if (n == 8) {
        return update_from_id(servinfo, table, msg);
    } else if (n != 8 + 12 * table->n)
        return E_BADMSG;

    recvd_dv = msg_unpack_dvec(msg, servid, table);
    if (!recvd_dv || recvd_dv->from == servid)
        return E_UNPACK;

    our_dv = table->costs;
    if (!our_dv || !table_lookup_server_by_id(table, recvd_dv->from))
        return E_LOOKUP;

    senderid = recvd_dv->from;
    if (!table_is_neighbor(table, senderid)) {
        fprintf(stderr, "recv'd message from non-neighbor, ignoring...\n");
        dvec_free(recvd_dv);
        return 0;
    }

    printf("RECEIVED A MESSAGE FROM SERVER %d\n", senderid);
    cost_to_sender = table_get_cost(table, senderid);
    assert(cost_to_sender >= 0);

    recvd_dvptr = recvd_dv->list->head;
    our_dvptr = our_dv->list->head;

    while (our_dvptr && recvd_dvptr) {
        our_dv_entry = our_dvptr->value;
        recvd_dv_entry = recvd_dvptr->value;

        cost = cost_to_sender + recvd_dv_entry->cost;
        if (our_dv_entry->cost > cost) {
            our_dv_entry->cost = cost;
            our_dv_entry->via = table_get_nexthop(table, senderid);
        }

        our_dvptr = our_dvptr->next;
        recvd_dvptr = recvd_dvptr->next;
    }

    reset_timeout(table, senderid);
    dvec_free(recvd_dv);
    return 0;
}

void
serv_perror(int errcode)
{
    if (errcode == E_SYSCALL)
        fprintf(stderr, "update error: system call\n");
    else if (errcode == E_BADMSG)
        fprintf(stderr, "update: partial message received, ignoring...");
    else if (errcode == E_UNPACK)
        fprintf(stderr, "update error: bad message format\n");
    else if (errcode == E_LOOKUP)
        fprintf(stderr, "update error: lookup failed\n");
    else if (errcode == E_BADADDR)
        fprintf(stderr, "update error: bad address\n");
    else if (errcode == E_PACK)
        fprintf(stderr, "update error: couldn't serialize message\n");
}
