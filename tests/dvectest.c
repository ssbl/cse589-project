#include <stdio.h>
#include "dvec.h"

int
main(void)
{
    char dv_str[MAXLEN_DVEC_STR];
    struct dvec *dv = dvec_init(1);

    dvec_add(dv, dvec_entry_new(3, 10));
    dvec_add(dv, dvec_entry_new(2, 8));
    dvec_update_cost(dv, 3, INF);

    puts("Printing string repr:");
    puts(dvec_str(dv, dv_str));

    dvec_print(dv);

    dvec_free(dv);
    return 0;
}
