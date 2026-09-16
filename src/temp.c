#include "snake.h"
#include<stdio.h>

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

int main(){
    Snake game;
    void render_state(SDL_Renderer *renderer, const Snake *game);
    snake_seed(time(NULL));
    snake_reset(&game);
    if (SDL_Init(SDL_INIT_VIDEO)!= 0){
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Snake Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GRID_WIDTH * CELL_SIZE,
        GRID_HEIGHT * CELL_SIZE,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL){
        printf("Window creation failed: %s\n", SDL_GetError());

        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    if (renderer == NULL){
        printf("Renderer creation failed: %s\n", SDL_GetError());

        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    int running = 1;
    SDL_Event event;
    Action action = ACTION_STRAIGHT;
    Uint32 last_move = SDL_GetTicks();
    int move_delay = 150;
    while (running && !snake_is_done(&game)){
        while (SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                running = 0;
            }

            if (event.type == SDL_KEYDOWN){
                switch (event.key.keysym.sym){
                    case SDLK_LEFT:
                    case SDLK_a:
                        action = ACTION_LEFT;
                        break;

                    case SDLK_RIGHT:
                    case SDLK_d:
                        action = ACTION_RIGHT;
                        break;

                    case SDLK_ESCAPE:
                        running = 0;
                        break;
                }
            }
        }

        Uint32 now = SDL_GetTicks();
        if (now - last_move >= (Uint32)move_delay){
            snake_step(&game, action);
            action = ACTION_STRAIGHT;
            last_move = now;
        }

        render_state(renderer, &game);
        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    snake_seed(42);
    snake_reset(&game);
    printf("<SNAKE GAME>\n");
    while (!snake_is_done(&game)) {
        int state[STATE_SIZE];
        snake_get_state(&game, state);
        render_state(renderer, &game);
        print_observation(state);
        Action action = get_action();
        float reward = snake_step(&game, action);
        printf("Reward: %.1f\n", reward);
    }
    render_state(renderer, &game);
    printf("GAME OVER\n");
    printf("Final score: %d\n", game.score);
    return 0;
}