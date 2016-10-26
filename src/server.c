#include "list.h"
#include "table.h"
#include "serventry.h"


struct servinfo *
servinfo_init(int id, int sockfd, time_t interval)
{
    struct servinfo *servinfo;

    if ((servinfo = malloc(sizeof(*servinfo))) == NULL)
        return NULL;

    servinfo->id = id;
    servinfo->sockfd = sockfd;
    servinfo->interval = interval;

    return servinfo;
}
