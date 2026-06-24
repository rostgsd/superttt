#include "sttt_out.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#define MAXLINE 48
static int board_len = 9;
static int x, y;

// draw game graphics
static void draw_horizontal_border() {
    printf("+---------+");
    printf("\033[%dD", board_len+2);
    printf("\033[%dB", 1);
}
void set_border(int position /*0 thru 8*/, char *color) {
    // horizontal and vertical cursor movement calculation
    x = position % 3;
    y = position / 3;
    // adjust according to board dimensions
    x *= board_len + 1;
    y *= 4;
    // move cursor to start
    printf("\033[%d;%dH", y+1, x+1);

    // select color
    char *colors[] = {"red", "green", "yellow", "blue", "magenta", "cyan"};
    for (size_t i = 0;i < sizeof(colors)/sizeof(char *);i++) {
        if (strcmp(color, colors[i]) == 0) {
            printf("\033[0;%zum", 31+i);
        }
    }
    // draw border
    draw_horizontal_border();
    for (int i = 0;i < 3;i++) {
        printf("|\033[%dC|", board_len);
        printf("\033[%dD", board_len+2);
        printf("\033[%dB", 1);
    }
    draw_horizontal_border();
    printf("\033[0m"); // color reset

    // move cursor back to start pos
    printf("\033[%dD", x);
    printf("\033[%dA", y+4); // 4 = height of board
    x = y = 0;
}
void set_game(int position, int *g) {
    // horizontal and vertical cursor movement calculation
    x = position % 3;
    y = position / 3;
    // adjust according to board dimensions
    x *= board_len + 1;
    y *= 4;
    // a (1,1) adjustment;
    x += 1; y += 1;
    // move cursor
    printf("\033[%d;%dH", y+1, x+1);

    // set game
    for (int i = 0;i < 9;i++) {
        int entry = g[i];
        int mod = (i + 1) % 2; // alternate light and dark
        if (entry == 1) { // X
            if (mod) printf("\033[31m X ");
            else printf("\033[31;47m X ");
        } else if (entry == -1) { // O
            if (mod) printf("\033[34m O ");
            else printf("\033[34;47m O ");
        } else { // blank
            if (mod) printf("   ");
            else printf("\033[47m   ");
        }
        // reset color tf
        printf("\033[0m");
        // nl after every 3 entry
        if ((i + 1) % 3 == 0) {
            printf("\033[%dD", board_len);
            printf("\033[%dB", 1);
            y += 1;
        }
    }

    // move cursor back to start pos
    printf("\033[%dD", x); printf("\033[%dA", y);
    x = y = 0;
}
void set_winner(int position, int winner) { // X or O
    // same logic in set_game
    x = (position % 3)*(board_len + 1) + 1;
    y = (position / 3)*4 + 1;
    printf("\033[%d;%dH", y+1, x+1);

    if (winner == 1 || winner == -1) {
        if (winner == 1) {
            printf("\033[41m   \033[0m   \033[41m   \n");
            printf("\033[%dC", x);
            printf("\033[0m   \033[41m   \033[0m   \n");
            printf("\033[%dC", x);
            printf("\033[41m   \033[0m   \033[41m   \n");
        } else {
            printf("\033[44m         \n");
            printf("\033[%dC", x);
            printf("  \033[0m     \033[44m  \n");
            printf("\033[%dC", x);
            printf("\033[44m         \n");
        }
        y += 3;
        printf("\033[0m"); // reset and move back
        printf("\033[%dA", y);
    }
    x = y = 0;
}
void set_empty_board() {
    int blank_game[] = {0,0,0, 0,0,0, 0,0,0,};
    for (int i = 0;i < 9;i++) {
        set_border(i, " ");
        set_game(i, blank_game);
    }
}
// cursor utils
void set_raw_mode(int enable) {
    // AI is pretty great lol
    static struct termios oldt, newt;
    if (enable) {
        // Get current terminal settings
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        // ICANON disables line buffering (wait for Enter)
        // ECHO disables printing the character back to the screen
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        // Restore old settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}
void clear_scr() {
    printf("\x1b[2J\x1b[H");
}
void move_cursor(int col, int row) {
    // negative
    if (col < 0) printf("\033[%dD", (-1)*col);
    if (row < 0) printf("\033[%dA", (-1)*row);
    // positive
    if (col > 0) printf("\033[%dC", col);
    if (row > 0) printf("\033[%dB", row);
}
void set_cursor(int pos_x, int pos_y) {
    printf("\033[%d;%dH", pos_y, pos_x);
}
void highlight(char *str) {
    printf("\033[30;47m%s\033[0m", str);
    move_cursor(-strlen(str), 0);
}
void confirm_highlight(char *str) {
    printf("\033[30;43m%s", str);
    fflush(stdout);
    usleep(130000);
    printf("\033[0m");
    move_cursor(-strlen(str), 0);
}
// game messages
void print_game_msg(char *msg, int row) {
    set_cursor(41, 9 + row);
    for (int i = 0;i <= MAXLINE;i++) putchar(' ');
    move_cursor(-MAXLINE, 0);
    printf("%s", msg);
    set_cursor(0, 0);
}
