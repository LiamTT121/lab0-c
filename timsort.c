#include <stdio.h>
#include <stdlib.h>

#include "list.h"
#include "timsort.h"

#define MIN_RUN_SIZE 5

struct run {
    struct list_head head;  // head of run
    struct list_head list;  // used to connect runs_queue
    size_t size;
};

struct runs_queue {
    struct list_head head;
    size_t count;
};

static struct run *new_run(struct list_head *head)
{
    struct run *run = calloc(1, sizeof(struct run));
    if (!run) {
        printf("Fail to construct new run\n");
        return NULL;
    }

    INIT_LIST_HEAD(&run->head);
    INIT_LIST_HEAD(&run->list);
    return run;
}

static void free_run(struct run *run)
{
    free(run);
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

static void free_runs_queue(struct runs_queue *rq)
{
    if (!rq)
        return;

    struct run *run, *next;
    list_for_each_entry_safe (run, next, &rq->head, list)
        free_run(run);
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
    struct list_head temp, *run1_head, *run2_head;

    INIT_LIST_HEAD(&temp);

    run1 = list_entry(head1, struct run, list);
    run2 = list_entry(head2, struct run, list);
    run1_head = &run1->head;
    run2_head = &run2->head;

    while (!list_empty(run1_head) && !list_empty(run2_head)) {
        struct list_head *cut;
        if (cmp(priv, run1_head->next, run2_head->next) <= 0)
            cut = run1_head->next;
        else
            cut = run2_head->next;

        list_del(cut);
        list_add_tail(cut, &temp);
    }

    list_splice_tail_init(run1_head, &temp);
    list_splice_tail(run2_head, &temp);
    list_splice_tail(&temp, run1_head);
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

    head = &all_run->head;
    while (all_run->count > 1) {
        int merge_count = all_run->count >> 1;
        for (left = head->next; merge_count-- > 0; left = left->next) {
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
    struct list_head *right;

    list_for_each (right, head) {
        if (cmp(priv, right, new_node) > 0)
            break;
    }
    list_del(new_node);
    list_add_tail(new_node, right);
}

static struct run *next_run(void *priv,
                            struct list_head *head,
                            list_cmp_func_t cmp)
{
    struct run *run;
    struct list_head *curr, *next;

    run = new_run(head);
    run->size = 1;

    curr = head->next;
    next = head->next->next;
    while (next != head && cmp(priv, curr, next) <= 0) {
        curr = next;
        next = next->next;
        run->size++;
    }
    list_cut_position(&run->head, head, curr);

    list_for_each_safe (curr, next, head) {
        if (run->size >= MIN_RUN_SIZE)
            break;

        insertion(priv, &run->head, curr, cmp);
        run->size++;
    }

    return run;
}

void timsort(void *priv, struct list_head *head, list_cmp_func_t cmp)
{
    struct runs_queue *all_run = new_runs_queue();
    struct run *run;

    while (!list_empty(head)) {
        run = next_run(priv, head, cmp);
        list_add_tail(&run->list, &all_run->head);
        if (++all_run->count < 4)
            continue;

        struct list_head *a, *b, *c, *d;
        struct run *ra, *rb, *rc, *rd;

        a = all_run->head.prev;
        b = all_run->head.prev->prev;
        c = all_run->head.prev->prev->prev;
        d = all_run->head.prev->prev->prev->prev;

        ra = list_entry(a, struct run, list);
        rb = list_entry(b, struct run, list);
        rc = list_entry(c, struct run, list);
        rd = list_entry(d, struct run, list);

        if (ra->size <= rb->size + rc->size ||
            rb->size <= rc->size + rd->size) {
            if (rb->size < rd->size)
                merge(priv, b, c, cmp);
            else
                merge(priv, c, d, cmp);

            all_run->count--;
        }
    }
    final_merge(priv, all_run, cmp);
    run = list_first_entry(&all_run->head, struct run, list);
    list_splice(&run->head, head);
    free_runs_queue(all_run);
}
