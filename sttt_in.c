// primary exacutables
#include "sttt_in.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <termios.h>
#include "sttt_out.h"
#include "menu.h"

// global game state
static int ttt_games[9][9];
static int sttt_game[9];
static int opponent, first_player = 0;
// game properties
static int current_game, end_of_turn, move_stage, turn;
static int squares_buf[9];

menu_t *init_game(unsigned int unused) {
    // set game to 0
    move_stage = current_game = 0;
    for (int i = 0;i < 9;i++) {
        sttt_game[i] = 0;
        for (int j = 0;j < 9;j++)
            ttt_games[i][j] = 0;
    }
    turn = first_player;
    set_empty_board();
    (turn == 0)? print_game_msg("X's turn", 0) : print_game_msg("O's turn", 0);
    return &moves;
}
menu_t *display_rules(unsigned int unused) {
    char *filename = "rules.txt";
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        clear_scr();
        fprintf(stderr, "File parse failure at display_rules");
        return &main_menu;
    }
    // display rules
    clear_scr();
    char stream_buf[64];
    while ( (fgets(stream_buf, sizeof(stream_buf), fp)) != NULL)
        printf("%s", stream_buf);
    fclose(fp);
    // exit
    set_raw_mode(1);
    while (getchar() != ' ')
        ;
    set_raw_mode(0);
    clear_scr();
    return &main_menu;
}
menu_t *set_opponent(unsigned int opp) {
    opponent = opp / 3;
    char *choices[] = {"local multiplayer", "dumb AI", "smart AI"};
    move_cursor(24, 1);
    printf("Opponent has been set to");
    // hacky way to delete the previous displayed choice
    for (int i = 0;i <= 24;i++)
        putchar(' ');
    move_cursor(-24, 0);
    printf("%s", choices[opponent]);
    set_cursor(0, 0);
    return &settings;
}
menu_t *set_team(unsigned int player) {
    first_player = player / 3;
    char choices[] = {'X', 'O'};
    move_cursor(24, 1);
    printf("Your team has been set to");
    for (int i = 0;i <= 24;i++)
        putchar(' ');
    move_cursor(-24, 0);
    printf("%c", choices[first_player]);
    set_cursor(0, 0);
    return &team;
}
menu_t *resume_game(unsigned int unused) {
    clear_scr();
    // relaod the game
    for (int i = 0;i < 9;i++) {
        set_border(i, " ");
        if (sttt_game[i] != 0) {
            set_winner(i, sttt_game[i]);
        } else {
            set_game(i, ttt_games[i]);
        }
    }
    // highlight current game
    if (move_stage == 1)
        set_border(current_game, "green");
    // display player's turn
    (turn % 2 == 0)? print_game_msg("X's turn", 0) : print_game_msg("O's turn", 0);
    return &moves;
}

static int win_conditions[8][3] = {
    {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {1, 4, 7}, {2, 5, 8}, {3, 6, 9}, {1, 5, 9}, {3, 5, 7}
};
static int check_4win(int *g) {
    int i;
    for (i = 0;i < 8;i++) {
        int v_1 = g[win_conditions[i][0]-1];
        int v_2 = g[win_conditions[i][1]-1];
        int v_3 = g[win_conditions[i][2]-1];
        if (v_1 != 0 && v_1 != 2 && v_1 == v_2 && v_1 == v_3) return v_1;
    }
    return 0; // 0, 1, or -1
}
static int decl_winner(int *g, int pos) {
    int board_winner, game_winner;
    // check board win
    if ((board_winner = check_4win(g)) == 0) return 0;
    set_winner(pos, board_winner);
    sttt_game[pos] = board_winner;
    // check game win
    if ((game_winner = check_4win(sttt_game)) == 0) return 0;
    // celebrate
    if (game_winner == 1) {
        print_game_msg("X wins the game", 2);
        for (int i = 0;i < 9;i++) set_border(i, "red");
    } else {
        print_game_msg("O wins the game", 2);
        for (int i = 0;i < 9;i++) set_border(i, "blue");
    }
    print_game_msg("Press space key to exit", 3);
    set_raw_mode(1);
    while (getchar() != ' ')
        ;
    clear_scr();
    set_raw_mode(0);
    return 1;
}
static int get_free_squares(int *g) {
    int buff_i = 0;
    for (int i = 0;i < 9;i++) {
        squares_buf[i] = 0;
        if (g[i] == 0) {
            squares_buf[buff_i] = i;
            buff_i++;
        }
    }
    return buff_i; // # of free spaces
}
static int find_potential_wins(int *g, int move_v) {
    int score = 0; // # of wins (2 in a row)
    for (int i = 0;i < 8;i++) {
        int truth_check = 0;
        int truth_table [3][3] = {
    {win_conditions[i][0], win_conditions[i][1], win_conditions[i][2]},
    {win_conditions[i][0], win_conditions[i][2], win_conditions[i][1]},
    {win_conditions[i][1], win_conditions[i][2], win_conditions[i][0]},
        };
        for (int j = 0;j < 3;j++) {
            if (move_v == g[truth_table[j][0]-1] && move_v == g[truth_table[j][1]-1]
                && g[truth_table[j][2]-1] == 0)
                truth_check++;
        }
        if (truth_check) score++;
    }
    return score;
}
static int score_g(int *g1, int *g2, int move_v) { // g2 dummy arg
    int best_score = -100;
    int n_free = get_free_squares(g1);
    int scores[n_free + 1];
    // local buf
    int local_squares_buf[9];
    for (int i = 0;i < 9;i++) local_squares_buf[i] = squares_buf[i];
    // score each move
    for (int i = 0;i < n_free;i++) {
        int sq = local_squares_buf[i];
        g1[sq] = move_v;
        int score = 0;
        if (check_4win(g1) == move_v) score += 20;
        score += find_potential_wins(g1, move_v)*2; // winning moves get 2x weight
        score -= find_potential_wins(g1, -move_v);
        scores[i] = score;
        g1[sq] = 0;
        // opponent score
        int opp_score;
        if (g2 != NULL) {
            if (sttt_game[sq] == 0) // if target is free
                opp_score = score_g(ttt_games[sq], NULL, -move_v);
            else opp_score = 10;
            scores[i] -= opp_score;
        }
        // find the highest score
        if (scores[i] > best_score) best_score = scores[i];
    }
    // single out canidates
    if (g2 == NULL) return best_score;

    int canidates[9];
    int choice = 0;
    for (int i = 0;i < n_free;i++) {
        if (scores[i] == best_score) canidates[choice++] = local_squares_buf[i];
    }
    return canidates[rand() % choice]; // move for board
}
static int game_is_tied(int *g, int curent_g, char scope) {
    for (int i = 0;i < 9;i++) if (g[i] == 0) return 0;
    if (scope == 'g') {
        sttt_game[curent_g] = 2;
    }
    if (scope == 'm') {
        print_game_msg("Game is a draw. There's no winner", 2);
        print_game_msg("Press space key to exit", 3);
        set_border(curent_g, " ");
        set_raw_mode(1);
        while (getchar() != ' ')
            ;
        set_raw_mode(0);
        clear_scr();
    }
    return 1;
}
// core logic
menu_t *advance_g(unsigned int square) {
    // decide whose turn it is
    int move_v = (turn % 2 == 0)? 1 : -1;
    // records user input
    if (move_stage == 0) { // game selectin
        if (sttt_game[square] != 0) { // validate
            print_game_msg("Game board already won, pick a different board", 1);
            return &moves;
        } else {
            print_game_msg(" ", 1);
        }
        // draw
        set_border(current_game, " ");
        current_game = square;
        set_border(current_game, "green");
        move_stage++;
        end_of_turn = 0;
    }
    else if (move_stage == 1) { // square selection
        if (ttt_games[current_game][square] != 0) { // validate
            if (ttt_games[current_game][square] == 2) {
                print_game_msg("Can't choose a tied game", 1);
                return &moves;
            }
            print_game_msg("Square is taken, retry selection", 1);
            return &moves;
        } else {
            print_game_msg(" ", 1);
        }
        ttt_games[current_game][square] = move_v;
        set_game(current_game, ttt_games[current_game]);
        // check for win
        if ((decl_winner(ttt_games[current_game], current_game)) == 1)
            return &main_menu;
        // check for tie
        game_is_tied(ttt_games[current_game], current_game, 'g');
        if (game_is_tied(sttt_game, current_game, 'm')) return &main_menu;
        // set the next target game
        if (sttt_game[square] != 0) { // if target is not free
            move_stage--;
            set_border(current_game, " ");
        } else {
            set_border(current_game, " ");
            current_game = square;
            set_border(current_game, "green");
        }
        end_of_turn = 1;
    }
    // decide if user turn is done
    if (end_of_turn) {
        turn++;
        print_game_msg(" ", 1);
        (turn % 2 == 0)? print_game_msg("X's turn", 0) : print_game_msg("O's turn", 0);
    }
    // check for ai
    if (opponent == 0 || end_of_turn == 0) return &moves;

    // ai turn
    int ai_square, ai_board, ind;
    move_v = (turn % 2 == 0)? 1 : -1;

    if (move_stage == 0) {
        if (opponent == 1) {
            ind = rand() % get_free_squares(sttt_game);
            ai_board = squares_buf[ind];
        } else ai_board = score_g(sttt_game, sttt_game, move_v);
        // draw
        set_border(current_game, " ");
        current_game = ai_board;
        set_border(current_game, "magenta");
    } else {
        set_border(current_game, "magenta");
    }
    // pause and stop spam
    fflush(stdout);
    set_raw_mode(1);
    sleep(1);
    tcflush(STDIN_FILENO, TCIFLUSH);
    set_raw_mode(0);
    // pick square
    if (opponent == 1) {
        ind = rand() % get_free_squares(ttt_games[current_game]);
        ai_square = squares_buf[ind];
    } else {
        ai_square = score_g(ttt_games[current_game], sttt_game, move_v);
    }
    // set game
    ttt_games[current_game][ai_square] = move_v;
    set_game(current_game, ttt_games[current_game]);
    // check for win
    if ((decl_winner(ttt_games[current_game], current_game)) == 1)
        return &main_menu;
    // check for tie
    game_is_tied(ttt_games[current_game], current_game, 'g');
    if (game_is_tied(sttt_game, current_game, 'm')) return &main_menu;
    // set the next target game
    if (sttt_game[ai_square] != 0) { // if target is not free
        move_stage = 0;
        set_border(current_game, " ");
    } else {
        move_stage = 1;
        set_border(current_game, " ");
        current_game = ai_square;
        set_border(current_game, "green");
    }
    // end ai turn
    turn++;
    print_game_msg(" ", 1);
    (turn % 2 == 0)? print_game_msg("X's turn", 0) : print_game_msg("O's turn", 0);
    return &moves;
}
