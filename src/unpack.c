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
    if (!inet_ntop(AF_INET, &ia, servaddr, ADDRLEN)) {
        perror("inet_ntop");
        return msgbuf;
    }

    assert(ptr - msgbuf == 8);
    return ptr;
}

static unsigned char *
unpack_entry(unsigned char *msgbuf, struct table *rcvrtable, struct dvec *dv)
{
    assert(msgbuf);
    assert(rcvrtable);
    assert(dv);

    uint32_t addr;
    uint16_t port, servid, cost;
    char addrstr[ADDRLEN], portstr[PORTLEN];
    unsigned char *ptr = msgbuf;
    struct in_addr ia;
    struct serventry *s_entry = NULL;

    ptr = unpack_uint32(ptr, &addr);
    ia.s_addr = addr;
    inet_ntop(AF_INET, &ia, addrstr, ADDRLEN);
    if (addrstr == NULL
        || !(s_entry = table_lookup_server_by_addr(rcvrtable, addrstr))) {
        fprintf(stderr, "\nunpack: malformed address from %d", s_entry->servid);
        return msgbuf;
    }

    ptr = unpack_uint16(ptr, &port);
    snprintf(portstr, PORTLEN, "%d", port);
    if (strcmp(s_entry->port, portstr)) {
        fprintf(stderr, "\nmalformed port number from %d", s_entry->servid);
        return msgbuf;
    }

    ptr = unpack_uint16(ptr, &servid); /* gobble 0 bytes */
    ptr = unpack_uint16(ptr, &servid);
    if (servid != s_entry->servid) {
        fprintf(stderr, "\ngot malformed servid, expected %d", s_entry->servid);
        return msgbuf;
    }

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
    assert(rcvrtable);
    assert(dv);

    unsigned char *msgptr = msgbuf;
    struct listitem *listptr = rcvrtable->servers->head;
    struct serventry *s_entry = NULL;

    while (listptr) {
        s_entry = listptr->value;
        msgptr = unpack_entry(msgptr, rcvrtable, dv);
        listptr = listptr->next;
    }

    if (msgptr - msgbuf != 12 * rcvrtable->n) {
        dvec_free(dv);
        dv = NULL;
    }

    return msgptr;
}

unsigned char *
msg_unpack_update(unsigned char *msgbuf, int *from, int *cost)
{
    assert(msgbuf);

    int zeros;
    unsigned char *ptr = msgbuf;

    ptr = unpack_uint16(ptr, from);
    ptr = unpack_uint16(ptr, cost);
    ptr = unpack_uint32(ptr, &zeros);

    if (ptr - msgbuf != 8)
        return NULL;

    return ptr;
}

/*
 * Deserialize a message into a distance vector structure.
 *
 * Expects n entries, where n is the number of entries in `rcvrtable`.
 * Caller must ensure that `msg` is at least (8 + 12*n) bytes.
 * Will likely cause a buffer overflow if this isn't checked.
 */

struct dvec *
msg_unpack_dvec(unsigned char *msg, int servid, struct table *rcvrtable)
{
    assert(msg);
    assert(rcvrtable);
    assert(rcvrtable->servers);

    uint16_t n, senderid;
    char servport[PORTLEN], servaddr[ADDRLEN];
    unsigned char *msgptr = msg;
    struct dvec *dv = NULL;     /* return value */
    struct serventry *s_entry = NULL;

    msgptr = unpack_header(msgptr, &n, servport, servaddr);
    if (!(s_entry = table_lookup_server_by_addr(rcvrtable, servaddr))
        || n != rcvrtable->n
        || s_entry->servid == servid) {
        fprintf(stderr, "recv'd a message from an unknown or malicious" \
                        "server, ignoring...\n");
        return NULL;
    }

    senderid = s_entry->servid;
    dv = dvec_init(senderid);

    msgptr = unpack_entries(msgptr, rcvrtable, dv);

    return dv;
}
