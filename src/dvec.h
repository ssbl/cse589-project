/* dvec.h -- distance vector data structure
 */

#ifndef DVEC_H_
#define DVEC_H_

#define INF 255
#define DIRECT 1
#define DEFAULT 0

#define MAXLEN_DVEC_ENTRY_STR 32
#define MAXLEN_DVEC_STR 128

#define E_LOOKUP_FAILED -1


struct dvec_entry {
    int to;
    int via;
    int cost;
    int direct_cost;
};

struct dvec {
    int from;
    struct list *list;
};


struct dvec_entry *dvec_entry_new(int to, int cost);
struct dvec *dvec_init(int from);
struct dvec *dvec_add(struct dvec *dvec, struct dvec_entry *dv_entry);
struct dvec *dvec_update_cost(struct dvec *dvec, int to, int cost, int flags);
int dvec_lookup(struct dvec *dvec, int to, int flags);
int dvec_lookup_via(struct dvec *dvec, int to);
char *dvec_entry_str(struct dvec_entry *dv_entry, char *dst);
char *dvec_str(struct dvec *dvec, char *dst);
void dvec_print(struct dvec *dvec);
void dvec_free(struct dvec *dvec);

#endif
