#include "snake.h"

int main() {
    snake_seed(42);
    Snake game;
    snake_reset(&game);
    while (!snake_is_done(&game)) {
        int state[STATE_SIZE];
        snake_get_state(&game, state);
        Action action = (Action)(rand() % 3);
        float reward = snake_step(&game, action);
        printf("Action: %d | Reward: %.1f\n", action, reward);
        print_state(&game);
    }
    printf("\nGame Over!\n");
    printf("Final score: %d\n", game.score);
    return 0;
}