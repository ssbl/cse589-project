#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "dvec.h"
#include "serventry.h"
#include "table.h"
#include "msg.h"
#include "topo.h"


static unsigned char * /* http://stackoverflow.com/q/1577161 */
pack_uint16(unsigned char *msgbuf, uint16_t value)
{
    msgbuf[0] = value >> 8;
    msgbuf[1] = value;

    return msgbuf + 2;
}

static unsigned char *
pack_uint32(unsigned char *msgbuf, uint32_t value)
{
    msgbuf[0] = value >> 24;
    msgbuf[1] = value >> 16;
    msgbuf[2] = value >> 8;
    msgbuf[3] = value;

    return msgbuf + 4;
}

static unsigned char *
pack_header(unsigned char *msgbuf, uint16_t n, uint16_t servport,
            uint32_t servaddr)
{
    assert(msgbuf);

    unsigned char *ptr = msgbuf;

    ptr = pack_uint16(ptr, n);
    ptr = pack_uint16(ptr, servport);
    ptr = pack_uint32(ptr, servaddr);

    return ptr;
}

static unsigned char *
pack_entry(unsigned char *msgbuf, uint16_t servid, struct table *table,
           struct serventry *s_entry)
{
    assert(msgbuf);
    assert(s_entry);

    unsigned char *msgptr = msgbuf;
    struct in_addr addr;

    if (inet_pton(AF_INET, s_entry->addr, &addr) <= 0)
        return msgbuf;
    msgptr = pack_uint32(msgptr, addr.s_addr);
    msgptr = pack_uint16(msgptr, atoi(s_entry->port));
    msgptr = pack_uint16(msgptr, 0);
    msgptr = pack_uint16(msgptr, s_entry->servid);
    msgptr = pack_uint16(msgptr,
                         table_get_cost(table, s_entry->servid));

    return msgptr;
}

static unsigned char *
pack_entries(unsigned char *msgbuf, int servid, struct table *table)
{
    assert(msgbuf);

    unsigned char *msgptr = msgbuf;
    struct listitem *ptr = table->servers->head;
    struct serventry *s_entry = NULL;

    while (ptr) {
        s_entry = ptr->value;
        msgptr = pack_entry(msgptr, servid, table, s_entry);
        ptr = ptr->next;
    }

    return msgptr;
}

unsigned char *
msg_pack_update(unsigned char *msgbuf, int id, int cost)
{
    assert(msgbuf);
    assert(cost >= 0);

    unsigned char *ptr = msgbuf;

    ptr = pack_uint16(ptr, (uint16_t) id);
    ptr = pack_uint16(ptr, (uint16_t) cost);
    ptr = pack_uint32(ptr, 0);  /* pad */

    if (ptr - msgbuf != 8)
        return NULL;

    return ptr;
}

/*
 * Create a message bytestring in the general message format.
 * Return a pointer to the message string if nothing fails.
 *
 * Returns NULL on failure.
 */

unsigned char *
msg_pack_dvec(int servid, struct table *table)
{
    assert(table);
    assert(table->servers);

    struct in_addr addr;
    unsigned char *ptr1, *ptr2;
    uint16_t n_entries, servport;
    uint32_t servaddr;

    struct serventry *s_entry = table_lookup_server_by_id(table, servid);
    if (!s_entry)
        return NULL;

    static unsigned char bcast_msg[MAXLEN_MSG]; /* return value */

    n_entries = table->n;
    servport = atoi(s_entry->port);
    if (inet_pton(AF_INET, s_entry->addr, &addr) <= 0) {
        perror("inet_pton");
        return NULL;
    }

    servaddr = addr.s_addr;
    ptr1 = pack_header(bcast_msg, n_entries, servport, servaddr);
    if (ptr1 - bcast_msg != 8)
        return NULL;

    ptr2 = pack_entries(ptr1, servid, table);
    if (ptr2 - ptr1 != 12 * table->n)
        return NULL;

    return bcast_msg;
}
