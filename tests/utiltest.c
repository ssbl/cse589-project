#include <stdio.h>

#include "utils.h"

int
main(void)
{
    struct sockaddr *ret = addr_from_ip("192.168.1.3", "80");

    if (ret)
        puts("Got an address");
    else
        puts("Got nothing");

    puts(get_localip());
    return 0;
}
