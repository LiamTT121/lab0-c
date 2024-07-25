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

static int move_record[N_GRIDS];
static int move_count = 0;

static struct termios orig_termios;

static void record_move(int move)
{
    move_record[move_count++] = move;
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
    printf("\n");
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

void cpu0(void *ptr)
{
    task_printf("0 start\n");
    int x;
    for (int i = 0; i < 1000000; i++) {
        x = i % 4243;
        if (i % 100000 == 0)
            task_printf("0: %02d%%\n", i / 10000);
    }
    task_printf("0 complete %d\n", x);
}

void cpu1(void *ptr)
{
    task_printf("1 start\n");
    int x;
    for (int i = 0; i < 1000000; i++) {
        x = i % 1234;
        if (i % 100000 == 0)
            task_printf("1: %02d%%\n", i / 10000);
    }
    task_printf("1 complete %d\n", x);
}

void disable_raw_mode(void)
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(void)
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

void read_raw_data(void *ptr)
{
    sigset_t sig_alarm;
    char c;

    sigemptyset(&sig_alarm);
    sigaddset(&sig_alarm, SIGALRM);
    while (1) {
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
            task_printf("press Ctrl-Q\n");
            c = '\0';
        } else if (c == CTRL_KEY('p')) {
            task_printf("press Ctrl-P\n");
            c = '\0';
        } else if (c == 'q')
            return;
    }
}

void cpu_battle()
{
    enable_raw_mode();
    timer_init();
    task_init();

    task_add(cpu0, "0");
    task_add(cpu1, "1");
    task_add(read_raw_data, "2");

    preempt_disable();
    my_timer_create(10000); /* 10000 microseconds == 10 milliseconds */
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
