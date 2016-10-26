#include <stdlib.h>

#include "list.h"
#include "table.h"
#include "server.h"
#include "serventry.h"
#include "utils.h"

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
    char *msg;
    socklen_t addrlen;
    struct sockaddr *servaddr;

    struct dvec *dv = table_get_dvec(table, servid);
    struct list *ptr = table->servers;
    struct serventry *s_entry = NULL;

    while (ptr) {
        s_entry = ptr->item;

        if (s_entry->id == servid || !s_entry->neighbor) {
            ptr = ptr->next;
            continue;
        }

        msg = msg_pack_dvec(dv); /* todo */
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

    ssize_t n;
    char dvec_from_neighbor[RECVLINES + 1];
    struct dvec *dv = NULL;

    n = recvfrom(sockfd, dvec_from_neighbor, RECVLINES, 0, NULL, NULL);
    if (n == -1) {
        perror("recvfrom");
        return E_SYSCALL;
    }

    dv = msg_parse_dvec(dvec_from_neighbor);
    if (dv == NULL)
        return E_PARSEMSG;

    /* todo */

    return 0;
}
