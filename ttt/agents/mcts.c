#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../fix_point.h"
#include "../game.h"
#include "mcts.h"
#include "util.h"

typedef uint32_t my_fix_point;

struct node {
    int move;
    char player;
    int n_visits;
    uint32_t score;
    struct node *parent;
    struct node *children[N_GRIDS];
};

static struct node *new_node(int move, char player, struct node *parent)
{
    struct node *node = malloc(sizeof(struct node));
    node->move = move;
    node->player = player;
    node->n_visits = 0;
    node->score = 0;
    node->parent = parent;
    memset(node->children, 0, sizeof(node->children));
    return node;
}

static void free_node(struct node *node)
{
    for (int i = 0; i < N_GRIDS; i++)
        if (node->children[i])
            free_node(node->children[i]);
    free(node);
}

static inline uint32_t uct_score(int n_total, int n_visits, uint32_t score)
{
    if (n_visits == 0)
        return FIX_POINT_MAX;

    uint32_t fix_log_total, fix_visits;
    fix_log_total = INT_TO_FIX_POINT(31 - __builtin_clz(n_total));
    fix_visits = INT_TO_FIX_POINT(n_visits);
    return fix_div(score, fix_visits) +
           fix_mul(EXPLORATION_FACTOR,
                   fix_sqrt(fix_div(fix_log_total, fix_visits)));
}

static struct node *select_move(struct node *node)
{
    struct node *best_node = NULL;
    uint32_t best_score = 0;
    for (int i = 0; i < N_GRIDS; i++) {
        if (!node->children[i])
            continue;
        uint32_t score = uct_score(node->n_visits, node->children[i]->n_visits,
                                   node->children[i]->score);
        if (!best_node || score > best_score) {
            best_score = score;
            best_node = node->children[i];
        }
    }
    return best_node;
}

static uint32_t simulate(const char *table, char player)
{
    char win;
    char current_player = player;
    char temp_table[N_GRIDS];
    memcpy(temp_table, table, N_GRIDS);
    while (1) {
        int *moves = available_moves(temp_table);
        if (moves[0] == -1) {
            free(moves);
            break;
        }
        int n_moves = 0;
        while (n_moves < N_GRIDS && moves[n_moves] != -1)
            ++n_moves;
        int move = moves[rand() % n_moves];
        free(moves);
        temp_table[move] = current_player;
        if ((win = check_win(temp_table)) != ' ')
            return calculate_win_value(win, player);
        current_player ^= 'O' ^ 'X';
    }
    return POINT_FIVE;
}

static void backpropagate(struct node *node, uint32_t score)
{
    while (node) {
        node->n_visits++;
        node->score += score;
        node = node->parent;
        score = INT_TO_FIX_POINT(1) - score;
    }
}

static void expand(struct node *node, const char *table)
{
    int *moves = available_moves(table);
    int n_moves = 0;
    while (n_moves < N_GRIDS && moves[n_moves] != -1)
        ++n_moves;
    for (int i = 0; i < n_moves; i++) {
        node->children[i] = new_node(moves[i], node->player ^ 'O' ^ 'X', node);
    }
    free(moves);
}

int mcts(const char *table, char player)
{
    char win;
    struct node *root = new_node(-1, player, NULL);
    for (int i = 0; i < ITERATIONS; i++) {
        struct node *node = root;
        char temp_table[N_GRIDS];
        memcpy(temp_table, table, N_GRIDS);
        while (1) {
            if ((win = check_win(temp_table)) != ' ') {
                uint32_t score =
                    calculate_win_value(win, node->player ^ 'O' ^ 'X');
                backpropagate(node, score);
                break;
            }
            if (node->n_visits == 0) {
                uint32_t score = simulate(temp_table, node->player);
                backpropagate(node, score);
                break;
            }
            if (node->children[0] == NULL)
                expand(node, temp_table);
            node = select_move(node);
            assert(node);
            temp_table[node->move] = node->player ^ 'O' ^ 'X';
        }
    }
    struct node *best_node = NULL;
    int most_visits = -1;
    for (int i = 0; i < N_GRIDS; i++) {
        if (root->children[i] && root->children[i]->n_visits > most_visits) {
            most_visits = root->children[i]->n_visits;
            best_node = root->children[i];
        }
    }
    int best_move = best_node->move;
    free_node(root);
    return best_move;
}
