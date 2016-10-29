/* msg.h --- message format
 */

#ifndef MSG_H_
#define MSG_H_

#define MAXLEN_MSG 128
#define TWOBYTELEN 3            /* 2 bytes + \0 character */
#define FOURBYTELEN 5           /* 4 bytes + \0 character */

unsigned char *msg_pack_dvec(int servid, struct table *table);

#endif
