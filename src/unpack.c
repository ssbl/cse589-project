#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "dvec.h"
#include "serventry.h"
#include "table.h"
#include "msg.h"
#include "topo.h"


static unsigned char *
unpack_uint16(unsigned char *msgbuf, uint16_t *value)
{
    *value = (msgbuf[0] << 8) + msgbuf[1];

    return msgbuf + 2;
}

static unsigned char *
unpack_uint32(unsigned char *msgbuf, uint32_t *value)
{
    *value = (msgbuf[0] << 24) + (msgbuf[1] << 16)
        + (msgbuf[2] << 8) + msgbuf[3];

    return msgbuf + 4;
}

static unsigned char *
unpack_header(unsigned char *msgbuf, uint16_t *n, char *servport,
              char *servaddr)
{
    assert(msgbuf);

    uint16_t port;
    uint32_t addr;
    struct in_addr ia;
    unsigned char *ptr = msgbuf;

    ptr = unpack_uint16(ptr, n);
    ptr = unpack_uint16(ptr, &port);
    ptr = unpack_uint32(ptr, &addr);

    snprintf(servport, PORTLEN, "%d", port);

    ia.s_addr = addr;
    if (!inet_ntop(AF_INET, &ia, servaddr, ADDRLEN))
        return msgbuf;

    printf("%d %s %s\n", *n, servaddr, servport);

    assert(ptr - msgbuf == 8);
    return ptr;
}

static unsigned char *
unpack_entry(unsigned char *msgbuf, struct serventry *s_entry,
             struct dvec *dv)
{
    assert(msgbuf);
    assert(s_entry);
    assert(dv);

    uint32_t addr;
    uint16_t port, servid, cost;
    char addrstr[ADDRLEN], portstr[PORTLEN];
    unsigned char *ptr = msgbuf;
    struct in_addr ia;

    ptr = unpack_uint32(ptr, &addr);
    ia.s_addr = addr;
    inet_ntop(AF_INET, &ia, addrstr, ADDRLEN);
    if (addrstr == NULL || strcmp(s_entry->addr, addrstr))
        return msgbuf;

    ptr = unpack_uint16(ptr, &port);
    snprintf(portstr, PORTLEN, "%d", port);
    if (strcmp(s_entry->port, portstr))
        return msgbuf;

    ptr = unpack_uint16(ptr, &servid); /* gobble 0 bytes */
    ptr = unpack_uint16(ptr, &servid);
    if (servid != s_entry->servid)
        return msgbuf;

    ptr = unpack_uint16(ptr, &cost);
    if (!dvec_add(dv, dvec_entry_new(servid, cost)))
        return msgbuf;

    assert(ptr - msgbuf == 12);
    return ptr;
}

static unsigned char *
unpack_entries(unsigned char *msgbuf, struct table *rcvrtable,
               struct dvec *dv)
{
    assert(msgbuf);

    unsigned char *msgptr = msgbuf;
    struct listitem *listptr = rcvrtable->servers->head;
    struct serventry *s_entry = NULL;

    while (listptr) {
        s_entry = listptr->value;
        msgptr = unpack_entry(msgptr, s_entry, dv);
        listptr = listptr->next;
    }

    assert(msgptr - msgbuf == 12 * rcvrtable->n);
    return msgptr;
}

/*
 * Deserialize a message into a distance vector structure.
 *
 * Expects n entries, returns NULL otherwise.
 */

struct dvec *
msg_unpack_dvec(unsigned char *msg, int servid, struct table *rcvrtable)
{
    assert(msg);
    assert(servid > 0 && servid <= MAXN);
    assert(rcvrtable);
    assert(rcvrtable->servers);

    uint16_t n, senderid;
    char servport[PORTLEN], servaddr[ADDRLEN];
    unsigned char *msgptr = msg;
    struct dvec *dv = NULL;     /* return value */
    struct serventry *s_entry = NULL;

    msgptr = unpack_header(msgptr, &n, servport, servaddr);
    if (!(s_entry = table_lookup_server_by_addr(rcvrtable, servaddr))
        || n != rcvrtable->n) {
        fprintf(stderr, "\nrecv'd message from unknown server, ignoring...");
        return NULL;
    }

    senderid = s_entry->servid;
    dv = dvec_init(senderid);

    msgptr = unpack_entries(msgptr, rcvrtable, dv);

    return dv;
}
