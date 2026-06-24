#pragma once
void set_border(int position /*0 thru 8*/, char *color);
void set_game(int position, int *g);
void set_winner(int position, int winner);
void set_empty_board();
// cursor utils
void set_raw_mode(int enable);
void clear_scr();
void move_cursor(int col, int row);
void set_cursor(int pos_x, int pos_y);
void highlight(char *str);
void confirm_highlight(char *str);
// game message
void print_game_msg(char *msg, int row);
