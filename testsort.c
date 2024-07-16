#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dudect/cpucycles.h"
#include "list.h"
#include "timsort.h"

#define ELE_SIZE 120

struct element {
    struct list_head list;
    int val;
};

int cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    return list_entry(a, struct element, list)->val -
           list_entry(b, struct element, list)->val;
}

static inline struct element *new_rand_element(void)
{
    struct element *e = malloc(sizeof(struct element));
    e->val = rand();
    return e;
}

static inline struct element *copy_element(const struct element *e)
{
    struct element *cp = malloc(sizeof(struct element));
    cp->val = e->val;
    return cp;
}

static void free_sample_data(struct list_head *head)
{
    struct element *curr, *next;
    list_for_each_entry_safe (curr, next, head, list)
        free(curr);
    free(head);
}

static struct list_head *copy_data(struct list_head *head)
{
    struct list_head *copy_head = malloc(sizeof(struct list_head));
    struct element *e, *copy_e;

    e = NULL;
    INIT_LIST_HEAD(copy_head);
    list_for_each_entry (e, head, list) {
        copy_e = copy_element(e);
        list_add_tail(&copy_e->list, copy_head);
    }
    return copy_head;
}

static struct list_head *create_sample_data(void)
{
    struct list_head *head = malloc(sizeof(struct list_head));
    struct element *e;
    int i;

    INIT_LIST_HEAD(head);
    for (i = 0; i < ELE_SIZE; i++) {
        e = new_rand_element();
        list_add_tail(&e->list, head);
    }
    return head;
}

/*
static void print_list(const struct list_head *head)
{
    struct element *curr, *next;
    list_for_each_entry_safe (curr, next, head, list)
        printf("%d, ", curr->val);
    printf("\n");

    int prev = -1, count = 0;
    list_for_each_entry_safe (curr, next, head, list) {
        if (curr->val < prev) {
            printf("Not sorted!\n");
            break;
        }
        count++;
    }
    printf("size: %d\n", count);
}
*/

int main(void)
{
    struct list_head *head, *copy1;

    srand(getpid() ^ getppid());

    head = create_sample_data();
    copy1 = copy_data(head);
    timsort(NULL, head, cmp);
    free_sample_data(head);
    free_sample_data(copy1);
    return 0;
}
