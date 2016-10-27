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
    servers->free = serventry_free;
    for (int i = 1; i <= n; i++)
        if (!parse_entry_to_list(line, fp, servers))
            return NULL;

    table_set_list(table, servers);
    return table;
}

int
main(void)
{
    struct table *table = parse_topofile("topofile");

    if (table) {
        printf("%d %d\n", table->n, table->neighbors);
        for (struct list *ptr = table->servers; ptr; ptr = ptr->next)
            printf("%s", serventry_str(ptr->item));
        table_free(table);
    } else
        puts("parse error");

    return 0;
}
