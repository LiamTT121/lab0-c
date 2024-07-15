#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "list.h"
// #include "timsort.h"

#define ELE_SIZE 15

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

static struct list_head *create_sample_data(void)
{
    struct list_head *head = malloc(sizeof(struct list_head));
    struct element *e;
    int i;

    INIT_LIST_HEAD(head);
    for (i = 0; i < ELE_SIZE; i++) {
        e = new_rand_element();
        list_add(&e->list, head);
    }
    return head;
}

static void free_sample_data(struct list_head *head)
{
    struct element *curr, *next;
    list_for_each_entry_safe (curr, next, head, list)
        free(curr);
    free(head);
}

static void print_list(const struct list_head *head)
{
    struct element *curr, *next;
    list_for_each_entry_safe (curr, next, head, list)
        printf("%d, ", curr->val);
    printf("\n");
}

int main(void)
{
    struct list_head *head;

    srand(getpid() ^ getppid());

    head = create_sample_data();
    print_list(head);
    free_sample_data(head);
    return 0;
}
