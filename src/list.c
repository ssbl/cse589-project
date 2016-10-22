#include <assert.h>
#include <stdlib.h>
#include "list.h"


struct list *
list_init()
{
    struct list *list;

    if ((list = malloc(sizeof(*list))) == NULL)
        return NULL;

    list->item = NULL;
    list->next = NULL;

    return list;
}

struct list *
list_add(struct list *list, void *item)
{
    assert(list);
    assert(item);

    struct list *ptr = list;

    while (ptr) {
        if (!ptr->item) {
            ptr->item = item;
            return list;
        } else if (!ptr->next) {
            ptr->next = list_init();
            ptr->next->item = item;
            return list;
        }
        ptr = ptr->next;
    }

    return NULL;
}

void
list_free(struct list *list)
{
    struct list *rmptr;
    struct list *ptr = list;

    while (ptr) {
        rmptr = ptr;
        ptr = ptr->next;

        free(rmptr->item);
        free(rmptr);
    }
}
