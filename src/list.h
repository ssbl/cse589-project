/* list.h -- generic singly linked list
 */

#ifndef LIST_H_
#define LIST_H_

struct listitem {
    void *value;
    struct listitem *next;
};

struct list {
    void (*free)(void *ptr);
    struct listitem *head;
};


struct listitem *listitem_new(void *value);
struct list *list_init(void);
struct list *list_add(struct list *list, void *item);
void list_free(struct list *list);

#endif
