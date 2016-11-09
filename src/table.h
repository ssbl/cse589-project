/* table.h -- routing table implementation
 */

#ifndef TABLE_H_
#define TABLE_H_

#define MAXN 8
#define MAX_NEIGHBORS 8
#define MAXLEN_TABLE_STR 256


struct table {
    int n;                             /* total servers */
    int id;                            /* id of this server */
    int neighbors;                     /* number of neighbors  */
    struct list *servers;              /* list of all servers */
    struct dvec *costs;                /* distance vectors */
};


struct table *table_init(int servid, int n, int neighbors);
struct table *table_set_list(struct table *table, struct list* servers);
int table_get_cost(struct table *table, int to);
struct serventry *table_lookup_server_by_id(struct table *table, int servid);
struct serventry *table_lookup_server_by_addr(struct table *table, char *addr);
struct table *table_update_cost(struct table *table, int to, int cost);
int table_is_neighbor(struct table *table, int servid);
void table_free(struct table *table);
char *table_str(struct table *table);
char *table_str_for_id(struct table *table);

#endif
