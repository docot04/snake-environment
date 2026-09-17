#include "snake.h"

#define ACTION_QUIT  4
#define ACTION_RESET 3

// send observation to python
// format: state[0]... state[10] reward score done[binary] 
static void send_observation(const Snake *game, float reward) {
    int state[STATE_SIZE];
    snake_get_state(game, state);
    printf("OK ");
    for (int i = 0; i < STATE_SIZE; i++) printf("%d ", state[i]);
    printf("%.2f %d %d\n", reward, game->score, game->done);
    fflush(stdout);
}

// send ERR to python
static void send_error() {
    printf("ERR\n");
    fflush(stdout);
}

// waits for an action from python while processing SDL events:
// returns 1 (action received), 0 (sdl quit), -1 (input error)
static int get_action(SDL_Window *window, int *action) {
    (void)window;
    while (1) {
        // process SDL events first
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) return 0;
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return 0;
        }

        // check whether python has sent anything through stdin (10ms wait before SDL reprocessing)
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


int main() {

    // initialize game
    Snake game;
    snake_seed((unsigned int)time(NULL));
    snake_reset(&game);

    // initialze SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        send_error();
        return 1;
    }
    if (TTF_Init() != 0){
    printf("TTF_Init failed: %s\n", TTF_GetError());
    SDL_Quit();
    return 1;
    }
    SDL_Window *window = SDL_CreateWindow(
        "Snake RL Environment",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GRID_WIDTH * CELL_SIZE,
        GRID_HEIGHT * CELL_SIZE,
        SDL_WINDOW_SHOWN
    );

    if (window == NULL) {
        send_error();
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        send_error();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",24);
    if (font == NULL){
        printf("Font loading failed: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // initial render
    render_state(renderer, &game, font);;
    send_observation(&game, 0.0f);

    // main loop
    bool running = true;
    while (running) {
        render_state(renderer, &game, font); // keep rendering current state
        int action;
        int result = get_action(window, &action); // get action from python
        if (result == 0) { // sdl window closed / ESC pressed
            printf("QUIT\n");
            fflush(stdout);
            break;
        }
        if (result < 0) { // stdin/select error
            send_error();
            break;
        }
        if (action == ACTION_QUIT) break; // quit
        if (action == ACTION_RESET) { // reset
            snake_reset(&game);
            render_state(renderer, &game, font);
            send_observation(&game, 0.0f);
            continue;
        }
        if (action != ACTION_STRAIGHT && action != ACTION_LEFT && action != ACTION_RIGHT) { // movement
            send_error();
            break;
        }
        if (!snake_is_done(&game)) { // dont move if dead
            float reward = snake_step(&game, (Action)action);
            render_state(renderer, &game, font);
            send_observation(&game, reward);
        }
        else {
            send_observation(&game, 0.0f); // python normally send RESET after done = 1
        }
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}