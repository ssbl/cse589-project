#include <stdlib.h>

#include "list.h"
#include "msg.h"
#include "serventry.h"
#include "table.h"
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

    int ret, servid = servinfo->id;
    unsigned char *msg;
    socklen_t addrlen;
    struct sockaddr *servaddr;

    struct listitem *ptr = table->servers->head;
    struct serventry *s_entry = NULL;

    msg = msg_pack_dvec(servid, table);

    while (ptr) {
        s_entry = ptr->value;

        if (s_entry->id == servid || !s_entry->neighbor) {
            ptr = ptr->next;
            continue;
        }

        servaddr = addr_from_ip(s_entry->addr);
        addrlen = sizeof(*servaddr);

        ret = sendto(servinfo->sockfd, msg, strlen(msg), 0, servaddr, addrlen);
        if (ret == -1)
            perror("sendto failed for server %d", s_entry->id);

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

    ret = shutdown(sockfd, SHUT_WR);
    if (ret == -1)
        perror("shutdown");

    return ret;
}

int
serv_update(struct servinfo *servinfo, struct table *table, struct dvec *dv)
{
    assert(servinfo);
    assert(table);
    assert(dv);

    int servid = servinfo->id, senderid;
    ssize_t n;
    char msg[RECVLINES + 1];
    struct dvec *recvd_dv = NULL, *our_dv = NULL;
    struct dvec_entry *recvd_dv_entry, *our_dv_entry;
    struct listitem *recvd_dvptr, *our_dvptr;

    /* todo: use a specific address, so that we don't receive it from anyone */
    n = recvfrom(sockfd, msg, RECVLINES, 0, NULL, NULL);
    if (n == -1) {
        perror("recvfrom");
        return E_SYSCALL;
    }

    recvd_dv = msg_unpack_dvec(msg, servid, table);
    if (!dv)
        return E_UNPACK;

    our_dv = table_get_dvec(table, servid);
    if (!our_dv)                /* we've made a big mistake */
        return E_LOOKUP;

    senderid = recvd_dv->from;
    recvd_dvptr = recvd_dv->list->head;
    our_dvptr = our_dv->list->head;

    while (our_dvptr && recvd_dvptr) { /* todo: add entries to dvec inorder */
        our_dv_entry = our_dvptr->value;
        recvd_dv_entry = recvd_dvptr->value;

        if (our_dv_entry->cost > recvd_dv_entry->cost) {
            our_dv_entry->cost = recvd_dv_entry->cost;
            our_dv_entry->via = senderid;
        }

        our_dvptr = our_dvptr->next;
        recvd_dvptr = recvd_dvptr->next;
    }

    return 0;
}
