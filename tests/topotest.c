#include <stdio.h>

#include "topo.h"    
#include "list.h"
#include "dvec.h"
#include "table.h"                             
#include "serventry.h"

int
main(void)
{
    struct table *table = parse_topofile("topofile");

    if (table) {
        printf("%d %d\n", table->n, table->neighbors);
        for (struct listitem *ptr = table->servers->head; ptr; ptr = ptr->next)
            printf("%s", serventry_str(ptr->value));
        printf("%s", table_str(table));
        puts(table_str_for_id(table));
        table_free(table);
    } else
        puts("parse error");

    return 0;
}
