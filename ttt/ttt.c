#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "agents/mcts.h"
#include "agents/negamax.h"
#include "agents/reinforcement_learning.h"
#include "coroutine.h"
#include "game.h"
#include "ttt.h"

#define CTRL_KEY(k) ((k) & 0x1f)

#define MAX_ROUND 50

static int move_record[N_GRIDS];
static int move_count = 0;
static bool show_board_flag = true;
static bool during_battle_flag = true;

static struct termios orig_termios;

static void record_move(int move)
{
    move_record[move_count++] = move;
}

static void print_time(void *dont_care)
{
    while (true) {
        time_t curr_time = time(NULL);
        const struct tm *tm = localtime(&curr_time);

        if (!during_battle_flag)
            return;
        preempt_disable();
        printf("\033[2K\033[A");
        printf("Current time: %02d:%02d:%02d\n", tm->tm_hour, tm->tm_min,
               tm->tm_sec);
        preempt_enable();
    }
}

static void print_moves(void)
{
    printf("Moves: ");
    for (int i = 0; i < move_count; i++) {
        printf("%c%d", 'A' + GET_COL(move_record[i]),
               1 + GET_ROW(move_record[i]));
        if (i < move_count - 1)
            printf(" -> ");
    }
    printf("\n\n");
}

static int get_input(char player)
{
    char *line = NULL;
    size_t line_length = 0;
    int parseX = 1;

    int x = -1, y = -1;
    while (x < 0 || x > (BOARD_SIZE - 1) || y < 0 || y > (BOARD_SIZE - 1)) {
        printf("%c> ", player);
        int r = getline(&line, &line_length, stdin);
        if (r == -1)
            exit(1);
        if (r < 2)
            continue;
        x = 0;
        y = 0;
        parseX = 1;
        for (int i = 0; i < (r - 1); i++) {
            if (isalpha(line[i]) && parseX) {
                x = x * 26 + (tolower(line[i]) - 'a' + 1);
                if (x > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    x = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // input does not have leading alphabets
            if (x == 0) {
                printf("Invalid operation: No leading alphabet\n");
                y = 0;
                break;
            }
            parseX = 0;
            if (isdigit(line[i])) {
                y = y * 10 + line[i] - '0';
                if (y > BOARD_SIZE) {
                    // could be any value in [BOARD_SIZE + 1, INT_MAX]
                    y = BOARD_SIZE + 1;
                    printf("Invalid operation: index exceeds board size\n");
                    break;
                }
                continue;
            }
            // any other character is invalid
            // any non-digit char during digit parsing is invalid
            // TODO: Error message could be better by separating these two cases
            printf("Invalid operation\n");
            x = y = 0;
            break;
        }
        x -= 1;
        y -= 1;
    }
    free(line);
    return GET_INDEX(y, x);
}

static void reset_game(char *table)
{
    preempt_disable();
    memset(table, ' ', N_GRIDS);
    memset(move_record, 0, N_GRIDS * sizeof(int));
    move_count = 0;
    preempt_enable();
}

/* using rl */
static void cpu_0(char *table, const rl_agent_t *agent)
{
    int move = play_rl(table, agent);
    record_move(move);
}

/* using negamax */
static void cpu_1(char *table, char symbol)
{
    int move = negamax_predict(table, symbol).move;
    table[move] = symbol;
    record_move(move);
}

static void game_process(void *dont_care)
{
    char table[N_GRIDS];
    char symbol_0 = 'X';
    char symbol_1 = 'O';
    char win;

    srand(time(NULL));

    /* prepare for reinforcement learning (cpu 0) */
    rl_agent_t rl_agent;
    unsigned int state_num = 1;
    CALC_STATE_NUM(state_num);
    init_rl_agent(&rl_agent, state_num, symbol_0);
    load_model(&rl_agent, state_num, MODEL_NAME);

    /* prepare for negamax (cpu1) */
    negamax_init();

    for (int round = 0; round < MAX_ROUND && during_battle_flag; round++) {
        reset_game(table);
        task_printf("--- New Game ---\n");

        while (during_battle_flag) {
            if (show_board_flag) {
                preempt_disable();
                draw_board(table);
                preempt_enable();
            }

            win = check_win(table);
            if (win != ' ')
                break;
            cpu_0(table, &rl_agent);

            win = check_win(table);
            if (win != ' ')
                break;
            cpu_1(table, symbol_1);
        }

        if (!during_battle_flag)
            return;

        if (show_board_flag) {
            preempt_disable();
            draw_board(table);
            preempt_enable();
        }

        if (win == 'D')
            task_printf("Game %d: It is a draw!\n\n", round + 1);
        else if (win == symbol_0)
            task_printf("Game %d: CPU 0 won!\n\n", round + 1);
        else
            task_printf("Game %d: CPU 1 won!\n\n", round + 1);

        preempt_disable();
        print_moves();
        preempt_enable();
    }
    preempt_disable();
    printf("Game is Over. Press Ctrl+Q to leave.\n\n");
    preempt_enable();
}

static void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static void enable_raw_mode(void)
{
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);

    raw = orig_termios;
    raw.c_iflag &= ~(IXON); /* disable Ctrl-Q and Ctrl-S */
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1; /* set timeout (100 milliseconds) for reading */
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void read_raw_data(void *dont_care)
{
    sigset_t sig_alarm;
    char c = '\0';

    sigemptyset(&sig_alarm);
    sigaddset(&sig_alarm, SIGALRM);
    while (during_battle_flag) {
        sigprocmask(SIG_BLOCK, &sig_alarm, NULL);
        int byte_read = read(STDIN_FILENO, &c, 1);
        sigprocmask(SIG_UNBLOCK, &sig_alarm, NULL);

        if (byte_read == -1) {
            preempt_disable();
            perror("read error");
            exit(1);
        }

        /* c will keep the value from the last valid read
         * need to reset c to prevent repeating outcomes
         */
        if (c == CTRL_KEY('q')) {
            preempt_disable();
            during_battle_flag = false;
            printf("Quit tic-tac-toe\n");
            preempt_enable();
            return;
        } else if (c == CTRL_KEY('p')) {
            preempt_disable();
            show_board_flag = !show_board_flag;
            preempt_enable();
            c = '\0';
        }
    }
}

static void cpu_battle()
{
    enable_raw_mode();
    timer_init();
    task_init();

    task_add(game_process, NULL);
    task_add(read_raw_data, NULL);
    task_add(print_time, NULL);

    preempt_disable();
    my_timer_create(10000); /* 10000 microseconds == 10 milliseconds */

    show_board_flag = true;
    during_battle_flag = true;

    /* coroutine */
    while (!list_empty(&task_main.list) || !list_empty(&task_reap)) {
        preempt_enable();
        timer_wait();
        preempt_disable();
    }
    preempt_enable();
    timer_cancel();
    disable_raw_mode();
}

void start_ttt(const bool is_cpu_vs_cpu)
{
    char table[N_GRIDS];
    char turn = 'X';
    char ai = 'O';
    int move;

    srand(time(NULL));
    memset(table, ' ', N_GRIDS);

    if (is_cpu_vs_cpu) {
        cpu_battle();
        return;
    }

    while (1) {
        char win = check_win(table);
        if (win == 'D') {
            draw_board(table);
            printf("It is a draw!\n");
            break;
        } else if (win != ' ') {
            draw_board(table);
            printf("%c won!\n", win);
            break;
        }

        if (turn == ai) {
            move = mcts(table, ai);
            if (move != -1) {
                table[move] = ai;
                record_move(move);
            }
        } else {
            draw_board(table);
            while (1) {
                move = get_input(turn);
                if (table[move] == ' ')
                    break;
                printf("Invalid operation: the position has been marked\n");
            }
            table[move] = turn;
            record_move(move);
        }
        turn = turn ^ 'O' ^ 'X';
    }

    print_moves();
}
