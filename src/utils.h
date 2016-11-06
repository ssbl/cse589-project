/* utils.h --- utility functions
 */

#ifndef UTILS_H_
#define UTILS_H_

#define RESOLV_IP "8.8.8.8"

struct sockaddr *addr_from_ip(char *addr, char *port);
char *get_localip(void);
int listen_socket(char *port);
int max(int a, int b);
int validate_strtol(char *s);

#endif
