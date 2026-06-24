#include <stdio.h>
#include <termios.h>
#include <unistd.h>

static int win_conditions[8][3] = {
    {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {1, 4, 7}, {2, 5, 8}, {3, 6, 9}, {1, 5, 9}, {3, 5, 7}
};
static int find_potential_wins(int *g, int move_v) {
    int score = 0;
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
int main() {
    int game[] = {
        1, 0, -1,
        1, 0, -1,
        -1, 1, 1
    };
    printf("Game score: %d\n", find_potential_wins(game, 1));
}
