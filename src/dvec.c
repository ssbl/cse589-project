#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dvec.h"
#include "list.h"


struct dvec_entry *
dvec_entry_new(int to, int cost)
{
    assert(to > 0 && cost >= 0);

    struct dvec_entry *dv_entry;

    if ((dv_entry = malloc(sizeof(*dv_entry))) == NULL)
        return NULL;

    dv_entry->to = to;
    dv_entry->via = to;
    dv_entry->cost = cost;

    return dv_entry;
}

struct dvec *
dvec_init(int from)
{
    struct dvec *dv;

    if ((dv = malloc(sizeof(*dv))) == NULL)
        return NULL;

    if ((dv->list = list_init()) == NULL) {
        free(dv);
        return NULL;
    }
    
    dv->from = from;
    return dv;
}

/*
 * Add an entry to the distance vector, ordered by id.
 */

struct dvec *
dvec_add(struct dvec *dvec, struct dvec_entry *dv_entry)
{
    assert(dvec);
    assert(dvec->list);
    assert(dv_entry);

    struct dvec_entry *ref = NULL;
    struct listitem *prev = NULL;
    struct listitem *new = listitem_new(dv_entry);
    struct listitem *ptr = dvec->list->head;

    while (ptr) {
        ref = ptr->value;
        if (ref->to >= dv_entry->to)
            break;
        prev = ptr;
        ptr = ptr->next;
    }

    if (prev)
        prev->next = new;
    else
        dvec->list->head = new;
    new->next = ptr;

    return dvec;
}

struct dvec *
dvec_update_cost(struct dvec *dvec, int to, int cost)
{
    assert(dvec);
    assert(dvec->list);
    assert(cost >= 0);
    assert(to > 0);

    struct listitem *ptr = dvec->list->head;
    struct dvec_entry *dv_entry;

    if (cost >= INF)
        cost = INF;

    while (ptr) {
        assert(ptr->value);
        dv_entry = ptr->value;
        if (dv_entry->to == to) {
            dv_entry->cost = cost;
            return dvec;
        }
        ptr = ptr->next;
    }

    return NULL;
}

int
dvec_lookup(struct dvec *dvec, int to)
{
    assert(dvec);
    assert(dvec->list);

    struct dvec_entry *dv_entry;
    struct listitem *ptr = dvec->list->head;

    while (ptr) {
        assert(ptr->value);
        dv_entry = ptr->value;

        if (dv_entry->to == to)
            return dv_entry->cost;

        ptr = ptr->next;
    }

    return E_LOOKUP_FAILED;
}

char *
dvec_entry_str(struct dvec_entry *dv_entry, char *dst)
{
    assert(dv_entry);
    assert(dst);

    char cost_str[4];

    int to = dv_entry->to;
    int via = dv_entry->via;
    int cost = dv_entry->cost;
    int len = MAXLEN_DVEC_ENTRY_STR;

    assert(to > 0 && cost >= 0 && cost <= INF);

    if (cost >= INF)
        strcpy(cost_str, "inf");
    else
        sprintf(cost_str, "%3d", cost);

    snprintf(dst, len, "--> %2d = %s via %2d\n", to, cost_str, via);

    return dst;
}

char *
dvec_str(struct dvec *dvec, char *dst)
{
    assert(dvec);
    assert(dst);

    char dv_entry_str[MAXLEN_DVEC_ENTRY_STR];

    int from = dvec->from;
    struct listitem *ptr = dvec->list->head;

    sprintf(dst, "from %2d:\n", from);
    while (ptr) {
        dvec_entry_str(ptr->value, dv_entry_str);
        strcat(dst, dv_entry_str);
        ptr = ptr->next;
    }

    return dst;
}

void
dvec_print(struct dvec *dvec)
{
    assert(dvec);

    char dv_str[MAXLEN_DVEC_STR];

    printf("%s", dvec_str(dvec, dv_str));
}

void
dvec_free(struct dvec *dvec)
{
    assert(dvec);

    list_free(dvec->list);
    free(dvec);
}
