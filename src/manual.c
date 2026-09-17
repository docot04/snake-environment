#include "snake.h"

int main(void) {

    // initialize game
    Snake game;
    snake_seed(time(NULL));
    snake_reset(&game);

    // initialize sdl2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    // initialize text
    if (TTF_Init() != 0){
        printf("TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
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
    if (window == NULL) {
        printf("Window creation failed: %s\n",SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // load font
    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",24);
    if (font == NULL){
    printf("Font loading failed: %s\n", TTF_GetError());
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 1;
    }

    // game loop
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            // Keyboard
            else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;

                // esc: exit
                if (key == SDLK_ESCAPE) {
                    running = false;
                }

                // S: reset game (only after snake dies)
                else if (key == SDLK_s) {
                    if (snake_is_done(&game)) {
                        snake_reset(&game);
                        printf("Game reset.\n");
                    }
                }

                // A (left), W (straight), D (right)
                else if (!snake_is_done(&game)) {
                    Action action;
                    switch (key) {
                        case SDLK_a: action = ACTION_LEFT; break;
                        case SDLK_w: action = ACTION_STRAIGHT; break;
                        case SDLK_d: action = ACTION_RIGHT; break;
                        default: continue;
                    }
                    float reward = snake_step(&game, action);
                    // printf("Action: %d | Reward: %.2f | Score: %d\n", action, reward, game.score);
                    if (snake_is_done(&game)) printf("GAME OVER! Press S to restart.\n");
                }
            }
        }

        // render
        render_state(renderer, &game, font);
        SDL_Delay(1);
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    return 0;
}