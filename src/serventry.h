/* serventry.h --- server functions
 */

#ifndef SERVENTRY_H_
#define SERVENTRY_H_

#define PORTLEN 8


struct serventry {
    int servid;
    int neighbor;
    char *port;
    char *addr;
};


struct serventry *serventry_new(int servid, char *addr, char *port);
struct serventry *serventry_new_localip(int servid, char *port);

#endif
