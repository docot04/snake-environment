#include "snake.h"
#include <stdio.h>

Action get_action(void) {
    char input;
    while (1) {
        printf("\nAction [A = left, S = straight, D = right]: ");
        scanf(" %c", &input);
        switch (input) {
            case 'a':
            case 'A': return ACTION_LEFT;
            case 's':
            case 'S': return ACTION_STRAIGHT;
            case 'd':
            case 'D': return ACTION_RIGHT;
            default: printf("Invalid input\n");
        }
    }
}

void print_observation(const int state[STATE_SIZE]) {
    printf("\nState: [");
    for (int i = 0; i < STATE_SIZE; i++) {
        printf("%d", state[i]);
        if (i < STATE_SIZE - 1) printf(", ");
    }
    printf("]\n");
}


int main() {
    snake_seed(42);
    Snake game;
    snake_reset(&game);
    printf("<SNAKE GAME>\n");
    while (!snake_is_done(&game)) {
        int state[STATE_SIZE];
        snake_get_state(&game, state);
        print_state(&game);
        print_observation(state);
        Action action = get_action();
        float reward = snake_step(&game, action);
        printf("Reward: %.1f\n", reward);
    }
    print_state(&game);
    printf("GAME OVER\n");
    printf("Final score: %d\n", game.score);
    return 0;
}