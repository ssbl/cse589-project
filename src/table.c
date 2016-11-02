#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dvec.h"
#include "list.h"
#include "serventry.h"
#include "table.h"


struct table *
table_init(int n, int neighbors)
{
    struct table *table;

    if ((table = malloc(sizeof(*table))) == NULL)
        return NULL;

    assert(n > 0);              /* decide a limit? */
    assert(neighbors > 0 && neighbors <= n);
    assert(neighbors <= MAX_NEIGHBORS);

    table->n = n;
    table->neighbors = neighbors;
    table->servers = list_init();

    for (int i = 1; i <= n; i++) {
        table->costs[i] = dvec_init(i);
        for (int j = 1; j <= n; j++) {
            if (i == j)
                dvec_add(table->costs[i], dvec_entry_new(j, 0));
            else
                dvec_add(table->costs[i], dvec_entry_new(j, INF));
        }
    }

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

struct table *
table_add_item(struct table *table, int from, int to, int cost)
{
    assert(table);
    assert(to > 0 && to <= MAX_NEIGHBORS);
    assert(from > 0 && from <= MAX_NEIGHBORS);

    struct dvec *dv = table->costs[from];
    struct dvec_entry *dv_entry = dvec_entry_new(to, cost);

    if (dvec_add(dv, dv_entry) == NULL)
        return NULL;

    return table;
}

int
table_get_cost(struct table *table, int from, int to)
{
    assert(table);
    assert(table->costs);
    assert(to > 0 && to <= MAXN);
    assert(from > 0 && from <= MAXN);

    return dvec_lookup(table->costs[from], to);
}

struct dvec *
table_get_dvec(struct table *table, int from)
{
    assert(table);
    assert(table->costs);
    assert(from > 0 && from <= MAXN);

    return table->costs[from];
}

struct serventry *
table_lookup_server_by_id(struct table *table, int servid)
{
    assert(table);
    assert(table->servers);
    assert(servid > 0 && servid <= MAXN);

    struct serventry *ret;
    struct listitem *serv = table->servers->head;

    while (((ret = serv->value) != NULL) && ret->servid != servid)
        serv = serv->next;

    return ret;
}

struct serventry *
table_lookup_server_by_addr(struct table *table, char *addr)
{
    assert(table);
    assert(table->servers);
    assert(addr);

    struct listitem *serv = table->servers->head;
    struct serventry *ret;

    while (((ret = serv->value) != NULL) && strcmp(ret->addr, addr) != 0)
        serv = serv->next;

    return ret;
}

struct table *
table_update_cost(struct table *table, int from, int to, int cost)
{
    assert(table);
    assert(to > 0 && to <= MAX_NEIGHBORS);
    assert(from > 0 && from <= MAX_NEIGHBORS);

    struct dvec *dv = table->costs[from];

    if (dvec_update_cost(dv, to, cost) == NULL)
        return NULL;

    return table;
}

void
table_free(struct table *table)
{
    assert(table);

    list_free(table->servers);
    for (int i = 1; i <= table->n; i++)
        dvec_free(table->costs[i]);

    free(table);
}

char *
table_str(struct table *table)
{
    assert(table);

    static char repr[MAXLEN_TABLE_STR]; /* return value */

    int mts = MAXLEN_TABLE_STR;
    int cost;
    char newline[2] = "\n";
    char tmp[MAXLEN_TABLE_STR];

    struct dvec_entry *dv_entry;
    struct listitem *list_ptr;

    memset(repr, 0, mts);

    strncat(repr, "X", mts);
    for (int i = 1; i <= table->n; i++) {
        snprintf(tmp, mts, "%4d", i);
        strncat(repr, tmp, mts);
    }
    strncat(repr, newline, mts);

    for (int i = 1; i <= table->n; i++) {
        list_ptr = table->costs[i]->list->head;

        snprintf(tmp, mts, "%d", i);
        strncat(repr, tmp, mts);

        while (list_ptr) {
            dv_entry = list_ptr->value;
            cost = dv_entry->cost;

            if (cost >= INF)
                snprintf(tmp, mts, " inf");
            else
                snprintf(tmp, mts, "%4d", cost);

            strncat(repr, tmp, mts);
            list_ptr = list_ptr->next;
        }

        strncat(repr, newline, mts);;
    }

    return repr;
}

char *
table_str_for_id(struct table *table)
{
    assert(table);

    int mts = MAXLEN_TABLE_STR;
    char cost[4];
    char tmp[MAXLEN_TABLE_STR];
    static char ret[MAXLEN_TABLE_STR];
    struct dvec *dv = table_get_dvec(table, table->id);
    struct listitem *ptr = dv->list->head;
    struct dvec_entry *dv_entry = NULL;

    strncat(ret, "DEST NEXTHOP COST\n", mts);

    while (ptr) {
        dv_entry = ptr->value;

        if (dv_entry->cost == INF)
            snprintf(cost, 4, "inf");
        else
            snprintf(cost, 4, "%3d", dv_entry->cost);

        snprintf(tmp, mts, "%4d %7d %4s\n",
                 dv_entry->to, dv_entry->via, cost);
        strncat(ret, tmp, mts);

        ptr = ptr->next;
    }

    return ret;
}
