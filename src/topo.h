/* topo.h --- topology file handling
 */

#ifndef TOPO_H_
#define TOPO_H_

#define MAXLEN_LINE 80

#define E_DUPE_IP -1
#define E_LOCALIP -2

struct table *parse_topofile(char *filename);

#endif
