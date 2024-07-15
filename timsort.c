#include <stdio.h>

#include "list.h"
#include "timsort.h"

#define MIN_RUN_SIZE 10

struct run {
    struct list_head head;
    size_t size;
};

struct runs_queue {
    struct list_head head;
    size_t count;
};

static struct run *new_run()
{
    struct run *r = calloc(1, sizeof(struct run));
    if (!r) {
        printf("Fail to construct new run\n");
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

static void merge(struct list_head *head1,
                  struct list_head *head2,
                  list_cmp_func_t cmp)
{
    struct list_head temp;

    INIT_LIST_HEAD(&temp);
    while (!list_empty(head1) && list_empty(head2)) {
        struct list_head *cut;
        if (cmp(head1->next, head2->next) <= 0)
            cut = head1->next;
        else
            cut = head2->next;

        list_add_tail(cut, &temp);
    }

    list_splice_tail(head1, &temp);
    list_splice_tail(head2, &temp);
    list_splice_tail(&temp, head1);
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

static struct run *next_run(struct list_head *head, list_cmp_func_t cmp)
{
    struct run *r;
    struct list_head *curr, *next;

    r = new_run();

    curr = head->next;
    next = head->next->next;
    while (next != head && cmp(curr, next) <= 0) {
        curr = next;
        next = next->next;
        r->size++;
    }
    list_cut_position(r->head, head, curr);

    list_for_each_safe (curr, next, head) {
        if (r->size >= MIN_RUN_SIZE)
            break;

        insertion(r->head, curr, cmp);
        r->size++;
    }

    return r;
}

static void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    struct runs_queue *all_run = new_runs_queue();

    /* just used to pass the static checking (unused variable)
     * need to clear this after implementation */
    all_run->count = 0;
    struct run delete_me;
    delete_me.size = 0;
    printf("timsort is wrong if you see this message. %zu\n", delete_me.size);
    /* clear until here */

    // todo;
}
