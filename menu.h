/* menu simulatiion contains menu objects with fixed array of member objects allocated.
 * Member objs return to menu directly or via function ptr. See implementation in main.c.
 */
#pragma once
#define MENU_SIZE 5

struct menu_item;
struct Menu;
typedef union selection_type {
    struct Menu *(*run_f)(unsigned int entry_code);
    struct Menu *next_menu;
} select_t;
// create menu objects
typedef struct menu_item {
    char *header; // for displaying on screen
    select_t directive;
    int is_active;
    int item_code; // entry code
    enum {
        EXEC,
        TRANSFER,
    } mode;
} member;
typedef struct Menu {
    // don't put anything past col 3
    member items[MENU_SIZE][MENU_SIZE];
    char *header;
    int abs_x;
    int abs_y;
    int item_w; // column offset
} menu_t;

extern menu_t main_menu, settings, team, moves, comfirm_quit;
