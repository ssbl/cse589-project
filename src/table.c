#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dvec.h"
#include "list.h"
#include "serventry.h"
#include "table.h"


struct table *
table_init(int servid, int n, int neighbors)
{
    struct table *table;

    if ((table = malloc(sizeof(*table))) == NULL)
        return NULL;

    assert(n > 0);              /* decide a limit? */
    assert(neighbors > 0 && neighbors <= n);
    assert(neighbors <= MAX_NEIGHBORS);

    table->n = n;
    table->id = servid;
    table->neighbors = neighbors;
    table->servers = list_init();
    table->costs = dvec_init(servid);

    if (!dvec_add(table->costs, dvec_entry_new(servid, 0)))
        return NULL;

    return table;
}

struct table *
table_set_list(struct table *table, struct list *list)
{
    assert(table);
    assert(list);

    list_free(table->servers);
    table->servers = list;
    list->free = serventry_free;
    return table;
}

int
table_get_cost(struct table *table, int to)
{
    assert(table);
    assert(table->costs);

    return dvec_lookup(table->costs, to, 0);
}

int
table_get_direct_cost(struct table *table, int to)
{
    assert(table);
    assert(table->costs);

    return dvec_lookup(table->costs, to, DIRECT);
}

struct serventry *
table_lookup_server_by_id(struct table *table, int servid)
{
    assert(table);
    assert(table->servers);

    struct serventry *ret;
    struct listitem *serv = table->servers->head;

    while (serv && ((ret = serv->value) != NULL) && ret->servid != servid)
        serv = serv->next;

    return serv ? ret : NULL;
}

int
table_get_nexthop(struct table *table, int to)
{
    assert(table);
    assert(table->costs);

    return dvec_lookup_via(table->costs, to);
}

struct serventry *
table_lookup_server_by_addr(struct table *table, char *addr)
{
    assert(table);
    assert(table->servers);
    assert(addr);

    struct listitem *serv = table->servers->head;
    struct serventry *ret;

    while (serv && ((ret = serv->value) != NULL) && strcmp(ret->addr, addr) != 0)
        serv = serv->next;

    return serv ? ret : NULL;
}

struct table *
table_update_cost(struct table *table, int to, int cost)
{
    assert(table);

    struct dvec *dv = table->costs;

    if (!dvec_update_cost(dv, to, cost, 0))
        return NULL;

    return table;
}

struct table *
table_update_topology(struct table *table, int to, int cost)
{
    assert(table);

    struct dvec *dv = table->costs;

    if (!dvec_update_cost(dv, to, cost, DIRECT))
        return NULL;

    return table;
}

int
table_is_neighbor(struct table *table, int servid)
{
    assert(table);

    struct serventry *s_entry = NULL;

    s_entry = table_lookup_server_by_id(table, servid);
    if (!s_entry)
        return 0;

    return s_entry->neighbor;
}

void
table_free(struct table *table)
{
    assert(table);

    list_free(table->servers);
    dvec_free(table->costs);

    free(table);
}

char *
table_str_for_id(struct table *table)
{
    assert(table);

    int mts = MAXLEN_TABLE_STR;
    char cost[4];
    char tmp[MAXLEN_TABLE_STR];
    static char ret_id_str[MAXLEN_TABLE_STR];
    struct dvec *dv = table->costs;
    struct listitem *ptr = dv->list->head;
    struct dvec_entry *dv_entry = NULL;

    memset(ret_id_str, 0, sizeof ret_id_str);
    strncat(ret_id_str, "DEST NEXTHOP COST\n", mts);

    while (ptr) {
        dv_entry = ptr->value;

        if (dv_entry->cost == INF)
            snprintf(cost, 4, "inf");
        else
            snprintf(cost, 4, "%3d", dv_entry->cost);

        snprintf(tmp, mts, "%4d %7d %4s\n",
                 dv_entry->to, dv_entry->via, cost);
        strncat(ret_id_str, tmp, mts);

        ptr = ptr->next;
    }

    return ret_id_str;
}
