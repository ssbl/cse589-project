/* serventry.h --- server functions
 */

#ifndef SERVENTRY_H_
#define SERVENTRY_H_

#define PORTLEN 8
#define ADDRLEN 64

struct serventry {
    int servid;
    int neighbor;
    char *port;
    char *addr;
    int lastrecvd;
};


struct serventry *serventry_new(int servid, char *addr, char *port);
struct serventry *serventry_new_localip(int servid, char *port);
char *serventry_str(struct serventry *s_entry);
void serventry_free(void *s_entry);

#endif
