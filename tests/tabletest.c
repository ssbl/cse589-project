#include <stdio.h>
#include "table.h"

int
main(void)
{
    struct table *table = table_init(6, 3);

    table->id = 1;
    table_update_cost(table, 1, 3, 20);

    /* printf("%s", table_str(table)); */
    printf("%s", table_str_for_id(table));

    table_free(table);
    return 0;
}
