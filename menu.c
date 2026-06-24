#include "menu.h"
// menu objects
menu_t main_menu = {
    .abs_x = 0,
    .abs_y = 0,
    .item_w = 16,
    .header = "Main Menu",
};
menu_t settings = {
    .abs_x = 0,
    .abs_y = 0,
    .item_w = 16,
    .header = "Select Opponent",
};
menu_t team = {
    .abs_x = 0,
    .abs_y = 0,
    .item_w = 16,
    .header = "Select X or O",
};
menu_t moves = {
    .abs_x = 42,
    .abs_y = 2,
    .item_w = 3,
    .header = "Make your next move"
};
menu_t comfirm_quit = {
    .abs_x = 0,
    .abs_y = 0,
    .item_w = 16,
    .header = "Are you sure you want to quit now?",
};
