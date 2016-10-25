#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dvec.h"
#include "list.h"
#include "table.h"


struct table *
table_init(int id, int n, int neighbors)
{
    struct table *table;

    if ((table = malloc(sizeof(*table))) == NULL)
        return NULL;

    assert(n > 0);              /* decide a limit? */
    assert(id > 0 && id <= n);
    assert(neighbors > 0 && neighbors <= n);
    assert(neighbors <= MAX_NEIGHBORS);

    table->id = id;
    table->n = n;
    table->neighbors = neighbors;
    table->servers = list_init();

    for (int i = 1; i <= n; i++) {
        table->costs[i] = dvec_init(i);
        for (int j = 1; j <= n; j++)
            dvec_add(table->costs[i], dvec_entry_new(j, INF));
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

struct table *
table_update_cost(struct table *table, int from, int to, int cost)
{
    assert(table);
    assert(to > 0 && to <= MAX_NEIGHBORS);
    assert(from > 0 && from <= MAX_NEIGHBORS);

    struct dvec *dv = table->costs[from];

    if (dvec_update(dv, to, cost) == NULL)
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
    struct list *list_ptr;

    memset(repr, 0, mts);

    strncat(repr, "X", mts);
    for (int i = 1; i <= table->n; i++) {
        snprintf(tmp, mts, "%4d", i);
        strncat(repr, tmp, mts);
    }
    strncat(repr, newline, mts);

    for (int i = 1; i <= table->n; i++) {
        list_ptr = table->costs[i]->list;

        snprintf(tmp, mts, "%d", i);
        strncat(repr, tmp, mts);

        while (list_ptr) {
            dv_entry = list_ptr->item;
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

int
main(void)
{
    struct table *table = table_init(1, 6, 3);

    table_update_cost(table, 1, 3, 20);

    printf("%s", table_str(table));

    table_free(table);
    return 0;
}
