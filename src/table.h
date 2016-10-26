/* table.h -- routing table implementation
 */

#ifndef TABLE_H_
#define TABLE_H_

#define MAX_NEIGHBORS 8
#define MAXLEN_TABLE_STR 256


struct table {
    int n;                             /* total servers */
    int id;                            /* id of this server */
    int neighbors;                     /* number of neighbors  */
    struct list *servers;              /* list of all servers */
    struct dvec *costs[MAX_NEIGHBORS]; /* distance vectors */
};


struct table *table_init(int n, int neighbors);
struct table *table_set_list(struct table *table, struct list* servers);
struct table *table_add_item(struct table *table, int from, int to, int cost);
struct table *table_update_cost(struct table *table, int from, int to, int cost);
void table_free(struct table *table);
char *table_str(struct table *table);

#endif
