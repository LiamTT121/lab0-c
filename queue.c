#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/* Create a new element  */
static element_t *new_element(const char *s)
{
    size_t len;
    element_t *e = malloc(sizeof(element_t));

    if (!e)
        return NULL;

    len = strlen(s) + 1;
    e->value = malloc(len * sizeof(char));

    if (!e->value) {
        free(e);
        return NULL;
    }

    memcpy(e->value, s, len);
    return e;
}

/* Free an element */
static void free_element(element_t *e)
{
    free(e->value);
    free(e);
}

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));

    if (!head)
        return NULL;

    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    struct list_head *curr, *next;

    if (!head)
        return;

    list_for_each_safe (curr, next, head) {
        element_t *e = list_entry(curr, element_t, list);
        free_element(e);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;

    e = new_element(s);
    if (!e)
        return false;

    list_add(&e->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    element_t *e;

    if (!head)
        return false;

    e = new_element(s);
    if (!e)
        return false;

    list_add_tail(&e->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e;

    if (!head || list_empty(head))
        return NULL;

    e = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&e->list);
    return e;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e;

    if (!head || list_empty(head))
        return NULL;

    e = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, e->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    list_del_init(&e->list);
    return e;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    int len;
    struct list_head *ptr;

    if (!head)
        return 0;

    len = 0;
    list_for_each (ptr, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    struct list_head *slow, *fast;

    if (!head || list_empty(head))
        return false;

    slow = fast = head->next;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    list_del(slow);
    free_element(list_entry(slow, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    /* assume the list is sorted */
    element_t *curr, *next;
    bool is_dup, next_is_dup;

    if (!head || list_empty(head))
        return false;

    is_dup = next_is_dup = false;
    list_for_each_entry_safe (curr, next, head, list) {
        if (&next->list != head)
            next_is_dup = strcmp(curr->value, next->value) == 0;

        if (is_dup || next_is_dup) {
            list_del(&curr->list);
            free_element(curr);
        }

        is_dup = next_is_dup;
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    struct list_head *prev, *ptr1, *ptr2;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    prev = head;
    ptr1 = head->next;
    ptr2 = head->next->next;
    while (ptr1 != head && ptr2 != head) {
        prev->next = ptr2;
        ptr2->prev = prev;
        ptr1->prev = ptr2;
        ptr1->next = ptr2->next;
        ptr2->next = ptr1;

        prev = ptr1;
        ptr1 = ptr1->next;
        ptr2 = ptr1->next;
    }

    ptr1->prev = prev;
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    struct list_head *prev, *curr, *next;

    if (!head || list_empty(head))
        return;

    prev = head;
    curr = head->next;
    while (curr != head) {
        next = curr->next;
        curr->next = prev;
        curr->prev = next;

        prev = curr;
        curr = next;
    }

    next = curr->next;
    curr->next = prev;
    curr->prev = next;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    struct list_head dummy, head_for_reverse;
    struct list_head *dummy_head, *tail, *next;
    int i;

    if (!head || list_empty(head) || k < 2)
        return;

    if (k == 2)
        return q_swap(head);

    dummy_head = &dummy;
    INIT_LIST_HEAD(dummy_head);
    list_splice_init(head, dummy_head);

    i = 0;
    list_for_each_safe (tail, next, dummy_head) {
        if (++i == k) {
            list_cut_position(&head_for_reverse, dummy_head, tail);
            q_reverse(&head_for_reverse);
            list_splice_tail(&head_for_reverse, head);
            i = 0;
        }
    }

    if (i > 0)
        list_splice_tail(dummy_head, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    struct list_head left_dummy, right_dummy;
    struct list_head *slow, *fast, *next;
    struct list_head *left, *right, *left_head, *right_head;
    const element_t *left_ele, *right_ele;

    if (!head || list_empty(head) || list_is_singular(head))
        return;

    slow = fast = head;
    while (fast->next != head && fast->next->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    left_head = &left_dummy;
    right_head = &right_dummy;
    INIT_LIST_HEAD(left_head);
    INIT_LIST_HEAD(right_head);

    list_cut_position(right_head, slow, head->prev);
    list_splice_init(head, left_head);

    q_sort(left_head, descend);
    q_sort(right_head, descend);

    left = left_head->next;
    right = right_head->next;
    left_ele = list_entry(left, element_t, list);
    right_ele = list_entry(right, element_t, list);

    while (left != left_head && right != right_head) {
        if ((strcmp(left_ele->value, right_ele->value) <= 0) ^ descend) {
            next = left->next;
            list_move_tail(left, head);
            left = next;
            if (left == left_head)
                break;
            left_ele = list_entry(left, element_t, list);
        } else {
            next = right->next;
            list_move_tail(right, head);
            right = next;
            if (right == right_head)
                break;
            right_ele = list_entry(right, element_t, list);
        }
    }

    if (!list_empty(left_head))
        list_splice_tail(left_head, head);
    if (!list_empty(right_head))
        list_splice_tail(right_head, head);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    struct list_head *curr, *prev;
    int count_node;

    if (!head)
        return 0;

    count_node = 0;
    for (curr = head->prev; curr != head; curr = prev) {
        const char *val_curr = list_entry(curr, element_t, list)->value;

        prev = curr->prev;
        while (prev != head) {
            const char *val_prev = list_entry(prev, element_t, list)->value;

            if (strcmp(val_curr, val_prev) >= 0)
                break;

            list_del(prev);
            free_element(list_entry(prev, element_t, list));
            prev = curr->prev;
        }

        count_node++;
    }
    return count_node;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    struct list_head *curr, *prev;
    int count_node;

    if (!head)
        return 0;

    count_node = 0;
    for (curr = head->prev; curr != head; curr = prev) {
        const char *val_curr = list_entry(curr, element_t, list)->value;

        prev = curr->prev;
        while (prev != head) {
            const char *val_prev = list_entry(prev, element_t, list)->value;

            if (strcmp(val_curr, val_prev) <= 0)
                break;

            list_del(prev);
            free_element(list_entry(prev, element_t, list));
            prev = curr->prev;
        }

        count_node++;
    }
    return count_node;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    queue_contex_t *curr, *next;
    struct list_head dummy, *dummy_head;
    int count;

    if (!head || list_empty(head))
        return 0;
    if (list_is_singular(head))
        return list_first_entry(head, queue_contex_t, chain)->size;

    dummy_head = &dummy;
    INIT_LIST_HEAD(dummy_head);
    count = 0;
    list_for_each_entry_safe (curr, next, head, chain) {
        count += curr->size;
        list_splice_init(curr->q, dummy_head);
        curr->size = 0;
    }

    q_sort(dummy_head, descend);

    curr = list_first_entry(head, queue_contex_t, chain);
    list_splice(dummy_head, curr->q);
    curr->size = count;
    return count;
}
