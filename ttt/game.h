#pragma once

#define BOARD_SIZE 4
#define GOAL 3
#define ALLOW_EXCEED 1
#define N_GRIDS (BOARD_SIZE * BOARD_SIZE)
#define GET_INDEX(i, j) ((i) * (BOARD_SIZE) + (j))
#define GET_COL(x) ((x) % BOARD_SIZE)
#define GET_ROW(x) ((x) / BOARD_SIZE)

#define FIX_POINT_MAX 0xFFFFFFFF
#define HALF_POINT (1U << (FRAC_DIGIT - 1))

#define for_each_empty_grid(i, table) \
    for (int i = 0; i < N_GRIDS; i++) \
        if (table[i] == ' ')

typedef uint32_t my_fix_point;

typedef struct {
    int i_shift, j_shift;
    int i_lower_bound, j_lower_bound, i_upper_bound, j_upper_bound;
} line_t;

extern const line_t lines[4];

int *available_moves(const char *table);
char check_win(const char *t);
my_fix_point calculate_win_value(char win, char player);
void draw_board(const char *t);
