#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

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
    s_entry->port = strdup(port);
    s_entry->addr = strdup(addr);

    return s_entry;
}

char *
serventry_str(struct serventry *s_entry)
{
    assert(s_entry);

    size_t total_len = INET6_ADDRSTRLEN + PORTLEN + 2;
    static char repr[INET6_ADDRSTRLEN];

    int servid = s_entry->servid;
    char *addr = s_entry->addr;
    char *port = s_entry->port;

    snprintf(repr, total_len, "%2d %16s %5s\n", servid, addr, port);

    return repr;
}

void
serventry_free(struct serventry *s_entry)
{
    assert(s_entry);

    free(s_entry->addr);
    free(s_entry->port);
    free(s_entry);
}

int
main(void)
{
    char port[PORTLEN] = "3453";
    char addr[INET6_ADDRSTRLEN] = "192.168.1.2";
    struct serventry *s_entry = serventry_new(1, addr, port);

    printf("%s", serventry_str(s_entry));

    serventry_free(s_entry);
    return 0;
}
