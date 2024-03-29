/* server.h --- server functions
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <sys/types.h>

#define RECVLINES 128
#define MAX_TIMEOUTS 3

#define E_SYSCALL -1
#define E_UNPACK -2
#define E_LOOKUP -3
#define E_BADMSG -4
#define E_BADADDR -5
#define E_PACK -6

struct servinfo {
    int id;
    int sockfd;
    int recvd;
    int is_alive;
    time_t interval;
};


struct servinfo *servinfo_init(int id, int sockfd, time_t interval);
int serv_broadcast(struct servinfo *servinfo, struct table *table);
int serv_crash(struct servinfo *servinfo);
int serv_update(struct servinfo *servinfo, struct table *table);
int serv_disable_neighbor(struct table *table, int neighbor_id);
int serv_send_update(struct servinfo *si, struct table *table, int to, int cost);
void serv_perror(int errcode);
void refresh_timeouts(struct servinfo *servinfo, struct table *table);

#endif
