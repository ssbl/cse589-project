/* list.h -- generic singly linked list
 */

#ifndef LIST_H_
#define LIST_H_

struct list {
    void *item;
    void (*free)(void *ptr);
    struct list *next;
};


struct list *list_init(void);
struct list *list_add(struct list *list, void *item);
void list_free(struct list *list);

#endif
