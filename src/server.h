/* server.h --- server functions
 */

#ifndef SERVER_H_
#define SERVER_H_

struct servinfo {
    int id;
    int sockfd;
    int packet_recvd;
    time_t interval;
};


struct servinfo *servinfo_init(int id, int sockfd, time_t interval);
int broadcast(struct servinfo *servinfo, struct table *table);
int crash(struct servinfo *servinfo);
int update(struct table *table, struct dvec *dv);

#endif
