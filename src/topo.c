#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "dvec.h"
#include "serventry.h"
#include "table.h"
#include "topo.h"
#include "utils.h"


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

static struct table *
parse_neighbor_entry(char *line, FILE *fp, struct table *table)
{
    assert(table);

    int servid, neighbor, cost;
    struct listitem *ptr;
    struct serventry *s_entry;

    line = fgets(line, MAXLEN_LINE, fp);
    if (!line)
        return NULL;

    if (sscanf(line, "%d%d%d", &servid, &neighbor, &cost) != 3)
        return NULL;

    assert(cost > 0);
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

    if (!table_update_cost(table, neighbor, cost))
        return NULL;

    return table;
}

static int
check_for_ip(struct list *servers)
{
    assert(servers);

    int servid = -1, count = 0;
    char *localip;
    struct serventry *s_entry = NULL;
    struct listitem *ptr = servers->head;

    localip = get_localip();
    if (!localip)
        return E_LOCALIP;

    while (ptr) {
        s_entry = ptr->value;
        if (!strcmp(s_entry->addr, localip)) {
            if (count == 1)
                return E_DUPE_IP;
            else {
                servid = s_entry->servid;
                count += 1;
            }
        }
        ptr = ptr->next;
    }

    return servid;
}

struct dvec *
create_cost_list(struct list *servers, struct table *table)
{
    assert(servers);
    assert(table);

    struct dvec *costs = table->costs;
    struct listitem *ptr = servers->head;

    while (ptr) {
        int *to = ptr->value;

        if (*to == table->id) {
            ptr = ptr->next;
            continue;
        }

        if (!dvec_add(costs, dvec_entry_new(*to, INF)))
            return NULL;

        ptr = ptr->next;
    }

    return costs;
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
    int n, neighbors, servid;
    char *localip, line[MAXLEN_LINE];
    struct list *servers = NULL;
    struct table *table = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen");
        return NULL;
    }

    if (!read_int(line, fp, &n) || !read_int(line, fp, &neighbors)) {
        fprintf(stderr, "expected 2 numbers, each on a separate line\n");
        return NULL;
    }

    if (n <= 0 || n > MAXN) {
        fprintf(stderr, "number out of range(8)\n");
        return NULL;
    } else if (neighbors < 0 || neighbors > MAXN || neighbors > n) {
        fprintf(stderr, "invalid neighbor count\n");
        return NULL;
    }

    localip = get_localip();
    if (!localip)
        goto fail;

    servers = list_init();
    for (int i = 0; i < n; i++)
        if (!parse_entry_to_list(line, fp, servers)) {
            fprintf(stderr, "server entry not in the correct format\n");
            goto fail;
        }

    if ((servid = check_for_ip(servers)) < 0) {
        fprintf(stderr, "couldn't find local entry\n");
        goto fail;
    }
    table = table_init(servid, n, neighbors);

    table_set_list(table, servers);
    if (!create_cost_list(servers, table))
        goto fail;
    for (int i = 0; i < neighbors; i++)
        if (!parse_neighbor_entry(line, fp, table)) {
            fprintf(stderr, "cost entry not in the correct format\n");
            goto fail;
        }

    fclose(fp);
    return table;

fail:
    if (table)
        table_free(table);
    return NULL;
}
