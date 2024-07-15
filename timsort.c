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

/*
 * @head1: one of header of list on runs_queue
 * @head2: another header of list on runs_queue
 *
 * two runs will be merged and link to head1,
 * then free the space of run #2.
 */
static void merge(void *priv,
                  struct list_head *head1,
                  struct list_head *head2,
                  list_cmp_func_t cmp)
{
    struct run *run1, *run2;
    struct list_head temp, *run_head1, *run_head2;

    INIT_LIST_HEAD(&temp);

    run1 = list_entry(head1, struct run, head);
    run2 = list_entry(head2, struct run, head);
    run_head1 = run1->head;
    run_head2 = run2->head;

    while (!list_empty(run_head1) && list_empty(run_head2)) {
        struct list_head *cut;
        if (cmp(priv, run_head1->next, run_head2->next) <= 0)
            cut = run_head1->next;
        else
            cut = run_head2->next;

        list_del(cur);
        list_add_tail(cut, &temp);
    }

    list_splice_tail(head1, &temp);
    list_splice_tail(head2, &temp);
    list_splice_tail(&temp, head1);
    run1->size += run2->size;

    list_del(head2);  // remove run #2 from all_run
    free_run(run2);
}

/*
 * suppose there are n runs, index are from 0, 1, ..., n-1.
 * keep merging (0 and n-1), (1 and n-2), and so on till only 1 run remaind.
 */
static void final_merge(void *priv,
                        struct runs_queue *all_run,
                        list_cmp_func_t cmp)
{
    struct list_head *head, *left, *right;

    head = all_run->head;
    while (all_run->count > 1) {
        for (left = head->next; left->next != head; left = left->next) {
            right = head->prev;
            merge(priv, left, right, cmp);
        }

        all_run->count = (all_run->count + 1) >> 1;
    }
}

static void insertion(void *priv,
                      struct list_head *head,
                      struct list_head *new_node,
                      list_cmp_func_t cmp)
{
    struct list_head *curr;

    list_for_each (curr, head) {
        if (cmp(priv, curr, new_node) > 0)
            break;
    }
    list_add_tail(new_node, curr);
}

static struct run *next_run(void *priv,
                            struct list_head *head,
                            list_cmp_func_t cmp)
{
    struct run *r;
    struct list_head *curr, *next;

    r = new_run();

    curr = head->next;
    next = head->next->next;
    while (next != head && cmp(priv, curr, next) <= 0) {
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

    while (!list_empty(head)) {
        struct run *run = next_run(head);
        list_add_tail(run->head, all_run->head);
        if (++all_run->count < 4)
            continue;

        struct list_head, *a, *b, *c, *d;
        struct run *ra, *rb, *rc, *rd;

        a = all_run->head->prev;
        b = all_run->head->prev->prev;
        c = all_run->head->prev->prev->prev;
        d = all_run->head->prev->prev->prev->prev;

        ra = list_entry(a, struct run, head);
        rb = list_entry(b, struct run, head);
        rc = list_entry(c, struct run, head);
        rd = list_entry(d, struct run, head);

        if (ra->size <= rb->size + rc->size ||
            rb->size <= rc->size + rd->size) {
            if (rb->size < rd->size)
                merge(priv, b, c, cmp);
            else
                merge(priv, c, d, cmp);
        }
    }

    final_merge(priv, all_run, cmp);
    free_runs_queue(all_run);
}
