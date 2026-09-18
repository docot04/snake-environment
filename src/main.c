#include "snake.h"

#define ACTION_RESET 3
#define ACTION_QUIT  4
#define FONT_PATH "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"

// send observation to python
// Format: OK state[0] state[1] ... state[STATE_SIZE-1] reward score done
static void send_observation(const Snake *game, float reward) {
    int state[STATE_SIZE];
    snake_get_state(game, state);
    printf("OK ");
    for (int i = 0; i < STATE_SIZE; i++) printf("%d ", state[i]);
    printf("%.2f %d %d\n",reward, game->score, game->done);
    fflush(stdout);
}

// Send error to Python
static void send_error() {
    printf("ERR\n");
    fflush(stdout);
}

// wait for an action from python while processing SDL events:
// returns 1 (action received), 0 (sdl quit), -1 (input error)
static int get_action(int *action, bool render) {
    while (1) {
        // only process SDL events if rendering is enabled
        if (render) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) return 0;
                if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return 0;
            }
        }

        // check stdin (10ms wait before SDL reprocessing)
        fd_set input;
        FD_ZERO(&input);
        FD_SET(STDIN_FILENO, &input);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        int result = select(STDIN_FILENO + 1, &input, NULL, NULL, &timeout);
        if (result < 0) return -1;
        if (result == 0) continue;

        // stdin has data
        if (fscanf(stdin, "%d", action) != 1) return -1;
        return 1;
    }
}

// INTERFACE MODE
static int run_interface_mode(Snake *game, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, bool render) {

    // initial render, then send initial state
    if (render) render_state(renderer, game, font);
    send_observation(game, 0.0f);
    bool running = true;
    while (running) {

        // Keep rendering current state
        if (render) render_state(renderer, game, font);
        int action;
        int result = get_action(&action, render);

        // SDL window closed / ESC pressed
        if (result == 0) {
            fflush(stdout);
            break;
        }

        // stdin/select error
        if (result < 0) {
            send_error();
            break;
        }

        // python requested quit
        if (action == ACTION_QUIT) break;

        // python requested reset
        if (action == ACTION_RESET) {
            snake_reset(game);
            if (render) render_state(renderer, game, font);
            send_observation(game, 0.0f);
            continue;
        }

        // invalid movement action
        if (action != ACTION_STRAIGHT && action != ACTION_LEFT && action != ACTION_RIGHT) {
            send_error();
            break;
        }

        // dont move if dead
        if (!snake_is_done(game)) {
            float reward = snake_step(game, (Action)action);
            if (render) render_state(renderer, game, font);
            send_observation(game, reward);
        }
        else {
            // python normally sends ACTION_RESET after done=1
            send_observation(game, 0.0f);
        }
    }
    return 0;
}

// MANUAL MODE
static int run_manual_mode(Snake *game, SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
    bool running = true;
    printf("Running Snake Game... \nMovement: A/W/D \nReset: S \nExit: ESC \n");
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            else if (event.type == SDL_KEYDOWN) {
                SDL_Keycode key = event.key.keysym.sym;

                // ESC: exit
                if (key == SDLK_ESCAPE) {
                    printf("Exiting Snake Game...\n");
                    running = false;
                }

                // S: reset after death
                else if (key == SDLK_s) {
                    if (snake_is_done(game)) {
                        snake_reset(game);
                        printf("Game reset.\n");
                    }
                }

                // movement
                else if (!snake_is_done(game)) {
                    Action action;
                    switch (key) {
                        case SDLK_a: action = ACTION_LEFT; break;
                        case SDLK_w: action = ACTION_STRAIGHT; break;
                        case SDLK_d: action = ACTION_RIGHT; break;
                        default: continue;
                    }

                    float reward = snake_step(game, action);
                    (void)reward;
                    if (snake_is_done(game)) printf("GAME OVER! Press S to restart.\n");
                }
            }
        }
        // render
        render_state(renderer, game, font);
        SDL_Delay(1);
    }
    return 0;
}

// SDL2 initialization
static int initialize_sdl(SDL_Window **window, SDL_Renderer **renderer, TTF_Font **font, const char *window_title) {
    // SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 0;
    }

    // SDL_ttf
    if (TTF_Init() != 0) {
        fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    // window
    *window = SDL_CreateWindow(
        window_title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GRID_WIDTH * CELL_SIZE,
        GRID_HEIGHT * CELL_SIZE,
        SDL_WINDOW_SHOWN
    );
    if (*window == NULL) {
        fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    // renderer
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL) {
        fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    // font
    *font = TTF_OpenFont(FONT_PATH, 24);
    if (*font == NULL) {
        fprintf(stderr, "Font loading failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }
    return 1;
}

// cleanup
static void cleanup_sdl(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font) {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

// main
int main(int argc, char *argv[]) {
    // ./snake manual
    // ./snake interface
    // ./snake interface --no-render
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Invalid Args\n");
        return 1;
    }

    bool interface_mode = false;
    bool render = true;
    if (strcmp(argv[1], "manual") == 0) interface_mode = false;
    else if (strcmp(argv[1], "interface") == 0) interface_mode = true;
    else {
        fprintf(stderr, "Invalid Args\n");
        return 1;
    }

    if (argc == 3) {
        if (strcmp(argv[2], "--headless") == 0) render = false;
        else {
            fprintf(stderr, "Invalid Args\n");
            return 1;
        }
    }

    // initialize game
    Snake game;
    snake_seed((unsigned int)time(NULL));
    snake_reset(&game);

    // SDL objects
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;

    // initialize SDL only if required
    if (!interface_mode || render) {
        const char *window_title;
        if (interface_mode) window_title = "Snake RL Environment";
        else window_title = "Snake Game";

        if (!initialize_sdl(&window, &renderer, &font, window_title)) {
            if (interface_mode) send_error();
            return 1;
        }
    }

    // run the selected mode
    int result;
    if (interface_mode) result = run_interface_mode(&game, window, renderer, font, render);
    else result = run_manual_mode(&game, window, renderer, font);
    
    // cleanup
    if (!interface_mode || render) cleanup_sdl(window, renderer, font);

    return result;
}
