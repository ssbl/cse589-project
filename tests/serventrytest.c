#include <stdio.h>

#include "serventry.h"

int
main(void)
{
    char port[PORTLEN] = "3453";
    char addr[ADDRLEN] = "192.168.1.2";
    struct serventry *s_entry = serventry_new(1, addr, port);

    printf("%s", serventry_str(s_entry));

    serventry_free(s_entry);
    return 0;
}
