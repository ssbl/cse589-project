#include <assert.h>
#include <stdlib.h>

#include "dvec.h"
#include "list.h"
#include "table.h"


struct table *
table_init(int id, int n, int neighbors)
{
    struct table *table;

    if ((table = malloc(sizeof(*table))) == NULL)
        return NULL;

    assert(n > 0);
    assert(id > 0 && id <= n);
    assert(neighbors > 0 && neighbors <= n);
    assert(neighbors <= MAX_NEIGHBORS);

    table->id = id;
    table->n = n;
    table->neighbors = neighbors;
    table->servers = list_init();

    for (int i = 1; i <= n; i++)
        table->costs[i] = dvec_init(i);

    return table;
}

struct table *
table_set_list(struct table *table, struct list *list)
{
    assert(table);
    assert(list);

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
table_str(struct table *table, char *dst)
{
    assert(table);

    return dst;
}

int
main(void)
{
    struct table *table = table_init(1, 6, 3);

    table_update_cost(table, 1, 3, 255);

    table_free(table);
    return 0;
}
