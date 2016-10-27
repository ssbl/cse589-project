#include <assert.h>
#include <stdlib.h>

#include "list.h"


struct listitem *
listitem_new(void *value)
{
    struct listitem *new;

    if ((new = malloc(sizeof(*new))) == NULL)
        return NULL;

    new->value = value;
    new->next = NULL;

    return new;
}

struct list *
list_init(void)
{
    struct list *list;

    if ((list = malloc(sizeof(*list))) == NULL)
        return NULL;

    list->head = NULL;
    list->free = NULL;

    return list;
}

struct list *
list_add(struct list *list, void *item)
{
    assert(list);
    assert(item);

    struct listitem *ptr = list->head;

    if (!list->head) {
        list->head = listitem_new(item);
        return list;
    }

    while (ptr) {
        if (!ptr->next) {
            ptr->next = listitem_new(item);
            return list;
        }
        ptr = ptr->next;
    }

    return NULL;
}

void
list_free(struct list *list)
{
    struct listitem *rmptr;
    struct listitem *ptr = list->head;

    while (ptr) {
        rmptr = ptr;
        ptr = ptr->next;

        if (list->free)
            list->free(rmptr->value);
        else
            free(rmptr->value);
        free(rmptr);
    }

    free(list);
}
