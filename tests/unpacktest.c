#include <assert.h>
#include <stdio.h>

#include "dvec.h"
#include "table.h"
#include "topo.h"
#include "msg.h"

int
main(void)
{
    unsigned char *msg;
    struct dvec *dv;
    struct table *table = parse_topofile("../tests/topofile");
    assert(table);

    msg = msg_pack_dvec(1, table);
    assert(msg);

    dv = msg_unpack_dvec(msg, 2, table);
    assert(dv);
    dvec_print(dv);

    table_free(table);
    dvec_free(dv);
    return 0;
}
