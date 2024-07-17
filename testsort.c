#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dudect/cpucycles.h"
#include "list.h"
#include "list_sort.h"
#include "timsort.h"

#define ELE_SIZE 10000
#define MAX_NAME_SIZE 64

static int array[ELE_SIZE];

struct element {
    struct list_head list;
    int val;
};

typedef void (*sample_func)(struct list_head *);

/*struct sample_type {
    sample_func func;
    char name[MAX_NAME_SIZE];
};*/

int cmp(void *priv, const struct list_head *a, const struct list_head *b)
{
    (*(int *) priv)++;
    return list_entry(a, struct element, list)->val -
           list_entry(b, struct element, list)->val;
}

int ascending_arr(const void *a, const void *b)
{
    return *(const int *) a - *(const int *) b;
}

static void free_list(struct list_head *head)
{
    struct element *curr, *next;
    list_for_each_entry_safe (curr, next, head, list)
        free(curr);
    free(head);
}

static void copy_array_to_list(struct list_head *head)
{
    struct element *curr;
    int i = 0;
    list_for_each_entry (curr, head, list) {
        if (i >= ELE_SIZE)
            return;
        curr->val = array[i++];
    }
}

static struct list_head *create_list(void)
{
    struct list_head *head = malloc(sizeof(struct list_head));
    int i;

    INIT_LIST_HEAD(head);
    for (i = 0; i < ELE_SIZE; i++) {
        struct element *e = malloc(sizeof(struct element));
        list_add_tail(&e->list, head);
    }
    return head;
}

static void gen_rand_array(void)
{
    int i;

    for (i = 0; i < ELE_SIZE; i++)
        array[i] = rand();
}

static inline void swap_array(const int i, const int j)
{
    int temp = array[i];
    array[i] = array[j];
    array[j] = temp;
}

static void reverse(int start, int end)
{
    while (start < end)
        swap_array(start++, end--);
}

static void shuffle_array(void)
{
    int i;

    i = 0;
    for (;;) {
        int j, n;
        n = rand() % 100;
        j = i + n;
        if (j >= ELE_SIZE)
            return;
        reverse(i, j);
        i = j + 1;
    }
}

int main(void)
{
    struct list_head *data1, *data2;
    int before, after, cmp_count;

    srand(getpid() ^ getppid());

    gen_rand_array();
    data1 = create_list();
    data2 = create_list();
    printf("timsort: cpu time, cmp count   |");
    printf("   list sort: cpu time, cmp count\n");

    copy_array_to_list(data1);
    copy_array_to_list(data2);

    cmp_count = 0;
    before = cpucycles();
    timsort(&cmp_count, data1, cmp);
    after = cpucycles();
    printf("   random : %d, %d   |   ", after - before, cmp_count);

    cmp_count = 0;
    before = cpucycles();
    list_sort(&cmp_count, data2, cmp);
    after = cpucycles();
    printf("%d, %d\n", after - before, cmp_count);

    qsort(array, ELE_SIZE, sizeof(int), ascending_arr);
    reverse(0, ELE_SIZE - 1);

    cmp_count = 0;
    before = cpucycles();
    timsort(&cmp_count, data1, cmp);
    after = cpucycles();
    printf("   sorted : %d, %d   |   ", after - before, cmp_count);

    cmp_count = 0;
    before = cpucycles();
    list_sort(&cmp_count, data2, cmp);
    after = cpucycles();
    printf("%d, %d\n", after - before, cmp_count);


    shuffle_array();
    copy_array_to_list(data1);
    copy_array_to_list(data2);

    cmp_count = 0;
    before = cpucycles();
    timsort(&cmp_count, data1, cmp);
    after = cpucycles();
    printf("sim practice : %d, %d   |   ", after - before, cmp_count);

    cmp_count = 0;
    before = cpucycles();
    list_sort(&cmp_count, data2, cmp);
    after = cpucycles();
    printf("%d, %d\n", after - before, cmp_count);

    struct element *e = NULL;
    int i;

    qsort(array, ELE_SIZE, sizeof(int), ascending_arr);
    i = 0;
    list_for_each_entry (e, data1, list) {
        if (array[i++] != e->val) {
            printf("Timsort Not Match!\n");
            break;
        }
    }

    i = 0;
    list_for_each_entry (e, data2, list) {
        if (array[i++] != e->val) {
            printf("Listsort Not Match!\n");
            break;
        }
    }

    free_list(data1);
    free_list(data2);
    return 0;
}
