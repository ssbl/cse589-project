/* server.h --- server functions
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <sys/types.h>

#define RECVLINES 128

#define E_SYSCALL -1
#define E_PARSEMSG -2

struct servinfo {
    int id;
    int sockfd;
    int recvd;
    time_t interval;
};


struct servinfo *servinfo_init(int id, int sockfd, time_t interval);
int serv_broadcast(struct servinfo *servinfo, struct table *table);
int serv_crash(struct servinfo *servinfo);
int serv_update(struct servinfo *servinfo, struct table *table, struct dvec *dv);

#endif
