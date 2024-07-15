#include <stdio.h>

#include "list.h"
#include "timsort.h"

#define MIN_RUN_SIZE 10

struct element {
    struct list_head list;
    int value;
};

struct run {
    struct list_head head;
    size_t size;
};

struct runs_queue {
    struct list_head head;
    size_t count;
};

static struct element *new_element(int value)
{
    struct element *e = malloc(sizeof(*e));
    if (!e) {
        printf("Fail to construct new element\n");
        return NULL;
    }

    INIT_LIST_HEAD(&e->list);
    e->value = value;
    return e;
}

static void free_element(struct element *e)
{
    free(e);
}

static struct run *new_run()
{
    struct run *r = calloc(1, sizeof(struct run));
    if (!r) {
        printf("Fail to construct new element\n");
        return NULL;
    }

    INIT_LIST_HEAD(&r->head);
    return r;
}

static free_run(struct run *r)
{
    free(r);
}

static struct runs_queue *new_runs_queue()
{
    struct runs_queue *rq = calloc(1, sizeof(*rq));

    if (!rq) {
        printf("Fail to construct new queue of runs\n");
        return NULL;
    }

    INIT_LIST_HEAD(&rq->head);
    return rq;
}

static free_runs_queue(struct runs_queue *rq)
{
    free(rq);
}

static int cmp_func(void *priv,
                    const struct list_head *a,
                    const struct list_head *b)
{
    // todo;
    return 0;
}

static void merge()
{
    // todo;
}

static void insertion(struct list_head *head,
                      struct list_head *new_node,
                      list_cmp_func_t cmp)
{
    struct list_head *curr;

    list_for_each (curr, head) {
        if (cmp(curr, new_node) > 0)
            break;
    }
    list_add_tail(new_node, curr);
}

static void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    struct runs_queue *all_run = new_runs_queue();

    /* just used to pass the static checking (unused variable)
     * need to clear this after implementation */
    all_run->count = 0;
    struct run delete_me;
    struct element delete_me_too;
    delete_me_too.value = 0;
    delete_me.size = delete_me_too.value;
    printf("timsort is wrong if you see this message. %zu\n", delete_me.size);
    /* clear until here */

    // todo;
}
