#include <assert.h>
#include <stdio.h>

#include "table.h"
#include "topo.h"
#include "msg.h"

int
main(void)
{
    unsigned char *buffer;
    struct table *table = parse_topofile("topofile");
    assert(table);

    buffer = msg_pack_dvec(1, table);
    assert(buffer);
    puts(table_str(table));

    for (int i = 0; i < 8 + 12 * table->n; i++)
        printf("%d", buffer[i]);
    printf("\n");

    table_free(table);
    return 0;
}
