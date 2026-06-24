#pragma once
// #include <stdio.h>
#include "menu.h"

menu_t *init_game(unsigned int unused);
menu_t *display_rules(unsigned int unused);
menu_t *set_opponent(unsigned int opp);
menu_t *set_team(unsigned int player);
menu_t *advance_g(unsigned int square);
menu_t *resume_game(unsigned int unused);
