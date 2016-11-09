#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "serventry.h"


struct serventry *
serventry_new(int servid, char *addr, char *port)
{
    struct serventry *s_entry;

    if ((s_entry = malloc(sizeof(*s_entry))) == NULL)
        return NULL;

    assert(servid > 0);
    assert(port);
    assert(addr);

    s_entry->servid = servid;
    s_entry->neighbor = 0;
    s_entry->lastrecvd = 0;
    s_entry->port = strdup(port);
    s_entry->addr = strdup(addr);

    return s_entry;
}

char *
serventry_str(struct serventry *s_entry)
{
    assert(s_entry);

    size_t total_len = ADDRLEN + PORTLEN + 2;
    static char repr[ADDRLEN];

    int servid = s_entry->servid;
    char *addr = s_entry->addr;
    char *port = s_entry->port;

    snprintf(repr, total_len, "%02d %16s %5s\n", servid, addr, port);

    return repr;
}

void
serventry_free(void *s_entry)   /* void * since it is used by list_free */
{
    assert(s_entry);

    struct serventry *ptr = s_entry;

    free(ptr->port);
    free(ptr->addr);
    free(ptr);
}
