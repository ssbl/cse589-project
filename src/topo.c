#include <assert.h>
#include <stdio.h>

#include "list.h"
#include "dvec.h"
#include "serventry.h"
#include "table.h"
#include "topo.h"


static inline int *
read_int(char *line, FILE *fp, int *n)
{
    line = fgets(line, MAXLEN_LINE, fp);
    if (!line)
        return NULL;

    if (sscanf(line, "%d", n) != 1)
        return NULL;

    return n;
}

static struct list *
parse_entry_to_list(char *line, FILE *fp, struct list *list) 
{
    int servid;
    char port[PORTLEN], addr[ADDRLEN];
    struct serventry *s_entry = NULL;

    line = fgets(line, MAXLEN_LINE, fp);
    if (!line)
        return NULL;

    if (sscanf(line, "%d%s%s", &servid, addr, port) != 3)
        return NULL;

    s_entry = serventry_new(servid, addr, port);
    list_add(list, s_entry);

    return list;
}

static struct dvec *
parse_neighbor_entry(char *line, FILE *fp, struct table *table)
{
    assert(table);
    int servid, neighbor, cost;
    struct dvec *dv;
    struct listitem *ptr;
    struct serventry *s_entry;

    line = fgets(line, MAXLEN_LINE, fp);
    if (!line)
        return NULL;

    if (sscanf(line, "%d%d%d", &servid, &neighbor, &cost) != 3)
        return NULL;

    assert(cost > 0);
    assert(servid > 0 && servid <= table->n);
    assert(neighbor > 0 && neighbor <= table->n);
    assert(neighbor != servid);

    ptr = table->servers->head;
    while (ptr) {
        s_entry = ptr->value;
        if (s_entry->servid == neighbor) {
            s_entry->neighbor = 1;
            break;
        }
        ptr = ptr->next;
    }

    dv = table->costs[servid];
    dv->from = servid;
    dvec_update_cost(dv, neighbor, cost);

    return dv;
}

/*
 * Read the topology file named `filename` from the current working directory
 * and create a new routing table.
 * Perform minimal semantic checking.
 *
 * This function fails if the file is not in the right format.
 * Returns NULL on failure.
 */

struct table *
parse_topofile(char *filename)
{
    assert(filename);

    FILE *fp;
    int n, neighbors;
    char line[MAXLEN_LINE];
    struct list *servers = NULL;
    struct table *table = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return NULL;
    }

    if (!read_int(line, fp, &n) || !read_int(line, fp, &neighbors))
        return NULL;

    if (n < 0 || n > MAXN)
        return NULL;
    else if (neighbors < 0 || neighbors > MAXN || neighbors > n)
        return NULL;

    table = table_init(n, neighbors);

    servers = list_init();
    for (int i = 1; i <= n; i++) {
        if (!parse_entry_to_list(line, fp, servers)) {
            table_free(table);
            return NULL;
        }
    }

    for (int i = 1; i <= neighbors; i++) {
        if (!parse_neighbor_entry(line, fp, table)) {
            table_free(table);
            return NULL;
        }
    }

    fclose(fp);
    table_set_list(table, servers);
    return table;
}
