/* Initialize member objs then inject into menus. Uses menus in a single state
 * machine to handle all program functionality.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include "sttt_out.h"
#include "menu.h"
#include "sttt_in.h"

// menu state machine
// more optimal if passing menu_t * but I don't care
void dispaly_menu(menu_t menu, char *menu_header) {
    // print menu headers
    set_cursor(menu.abs_x, menu.abs_y);
    printf("%s", menu_header);
    move_cursor(-strlen(menu_header), 1);
    // print menu items
    move_cursor(0, 1);
    for (int i = 0;i < MENU_SIZE;i++) {
        int y_offset = 0;
        for (int j = 0;j < MENU_SIZE;j++) {
            member menu_item = menu.items[i][j];
            if (menu_item.is_active == 1) {
                printf("%s", menu_item.header);
                move_cursor(-strlen(menu_item.header), 1);
            } else {
                move_cursor(0, 1);
            }
            y_offset++;
        }
        move_cursor(menu.item_w, -y_offset);
    }
    set_cursor(0,0);
}
member make_selection(menu_t *menu, int selec_x, int selec_y) {
    set_raw_mode(1);
    int ch;
    // goto default selection (don't bother w abstracting)
    dispaly_menu(*menu, menu->header);
    set_cursor(menu->abs_x, menu->abs_y);
    move_cursor(0, 2);
    move_cursor(menu->item_w * selec_x, selec_y);
    highlight(menu->items[selec_x][selec_y].header);

    while ((ch = getchar()) != ' ') {
        int selec_xbuf = selec_x;
        int selec_ybuf = selec_y;
        // render the menu
        dispaly_menu(*menu, menu->header);
        // Handle input
        switch (ch) {
            case 'a': case 'A': if (selec_x > 0) selec_xbuf--; break;
            case 'd': case 'D': if (selec_x < MENU_SIZE - 1) selec_xbuf++; break;
            case 'w': case 'W': if (selec_y > 0) selec_ybuf--; break;
            case 's': case 'S': if (selec_y < MENU_SIZE - 1) selec_ybuf++; break;
        }
        if (menu->items[selec_xbuf][selec_ybuf].is_active == 1) { // validate
            selec_x = selec_xbuf;
            selec_y = selec_ybuf;
        }
        // go to selection
        set_cursor(menu->abs_x, menu->abs_y);
        move_cursor(0, 2); // set specific amount later
        move_cursor(menu->item_w * selec_x, selec_y);
        highlight(menu->items[selec_x][selec_y].header);
    }
    // calculate the item code
    menu->items[selec_x][selec_y].item_code = (selec_y*3) + selec_x;

    confirm_highlight(menu->items[selec_x][selec_y].header);
    // wrap up
    set_raw_mode(0);
    set_cursor(0, 0);
    return menu->items[selec_x][selec_y];
}
// menu items
member main_comp[] = {
    {.header = "Play", .mode = EXEC, .directive.run_f = init_game},
    {.header = "Settings", .mode = TRANSFER, .directive.next_menu = &settings},
    {.header = "How to Play", .mode = EXEC, .directive.run_f = display_rules},
    {.header = "Teams", .mode = TRANSFER, .directive.next_menu = &team},
    {.header = "Quit Game", .mode = EXEC, .directive.run_f = NULL},
    {NULL},
};
member settings_comp[] = {
    {.header = "Local multiplayer", .mode = EXEC, .directive.run_f = set_opponent},
    {.header = "Dumb AI", .mode = EXEC, .directive.run_f = set_opponent},
    {.header = "Smart AI", .mode = EXEC, .directive.run_f = set_opponent},
    {.header = "Back", .mode = TRANSFER, .directive.next_menu = &main_menu},
    {NULL},
};
member team_comp[] = {
    {.header = " X ", .mode = EXEC, .directive.run_f = set_team},
    {.header = " O ", .mode = EXEC, .directive.run_f = set_team},
    {.header = "Back", .mode = TRANSFER, .directive.next_menu = &main_menu},
    {NULL},
};
member quit_comp[] = {
    {.header = "No", .mode = EXEC, .directive.run_f = resume_game},
    {.header = "Yes", .mode = TRANSFER, .directive.next_menu = &main_menu},
    {NULL},
};
// add items to menu
int menu_append(menu_t *menu, int pos_x, int pos_y, member item) {
    if (pos_x < 0 || pos_y < 0 || pos_x > MENU_SIZE-1 || pos_y > MENU_SIZE-1) {
        fprintf(stderr, "Unable to add menu item %s for %s at (%d,%d)\n", item.header, menu->header, pos_x, pos_y);
        return 1;
    }
    menu->items[pos_x][pos_y] = item;
    menu->items[pos_x][pos_y].is_active = 1;
    return 0;
}
int prog_init() {
    // menu comp
    // moves buttons
    int errors = 0;
    char *square_labels[] = {" 1 ", " 2 ", " 3 ", " 4 ", " 5 ", " 6 ", " 7 ", " 8 ", " 9 ",};
    for (int i = 0;i < 9;i++) {
        member square = {.header = square_labels[i], .mode = EXEC, .directive.run_f = advance_g};
        if ((menu_append(&moves, i % 3, i / 3, square)) == 1)
            errors++;
    }
    // add ingame quit
    member quit_game = {.header = "Quit", .mode = TRANSFER, .directive.next_menu = &comfirm_quit};
    if ((menu_append(&moves, 0, 3, quit_game)) == 1)
        errors++;
    // menu items
    menu_t *menus[] = {&main_menu, &settings, &team, &comfirm_quit};
    member *menu_items[] = {main_comp, settings_comp, team_comp, quit_comp};
    for (int i = 0;i < 4;i++) {
        int j = 0;
        while (menu_items[i][j].header) {
            if ((menu_append(menus[i], 0, j, menu_items[i][j])) == 1)
                errors++;
            j++;
        }
    }
    return errors;
}
int main() {
    // init
    int menu_errs;
    if ( (menu_errs = prog_init()) ) {
        fprintf(stderr, "%d menu init problems found\n", menu_errs);
        exit(1);
    }
    // seed
    srand(time(NULL));

    // main loop
    clear_scr();
    menu_t *current_menu = &main_menu;
    member choice = make_selection(current_menu, 0, 0);
    int selec_x, selec_y; // to save last input for &moves
    while (current_menu) {
        // process choice
        if (choice.mode == EXEC) {
            if (!choice.directive.run_f) break;
            current_menu = choice.directive.run_f(choice.item_code);
            selec_x = choice.item_code % 3;
            selec_y = choice.item_code / 3;
            // return to main menu exception
            if (current_menu == &main_menu) selec_x = selec_y = 0;
        } else if (choice.mode == TRANSFER) {
            current_menu = choice.directive.next_menu;
            selec_x = selec_y = 0;
            clear_scr();
        }
        // make next choice
        choice = make_selection(current_menu, selec_x, selec_y);
    }
    clear_scr();
}
