#include "snake.h"

// HELPER FUNCTIONS

/**
 * ACTION: checks if two coordinates are equal
 * PARAMS: Position, Position
 * RETURN: bool
 */
static bool position_equal(Position pos1, Position pos2) {
    return (pos1.x == pos2.x && pos1.y == pos2.y);
}

/**
 * ACTION: check if a coordinate is inside allowed grid
 * PARAMS: Position
 * RETURN: bool
 */
static bool position_is_inside_grid(Position pos) {
    bool result = (pos.x>=0 && pos.x<GRID_WIDTH && pos.y>=0 && pos.y<GRID_HEIGHT);
    return result;
}

/**
 * ACTION: check if snake exists in that coordinate
 * PARAMS: Snake, Position
 * RETURN: bool
 */
static bool snake_contains_position(const Snake *game, Position pos) {
    for (int i = 0; i < game->length; i++) {
        if (position_equal(game->snake[i], pos)) return true;
    }
    return false;
}

/**
 * ACTION: get next position of the snake's head after current state
 * PARAMS: Snake, Direction
 * RETURN: Position
 */
static Position get_next_position(const Snake *game, Direction direction) {
    Position head = game->snake[0];
    switch (direction) {
        case DIRECTION_UP: head.y--; break;
        case DIRECTION_DOWN: head.y++; break;
        case DIRECTION_LEFT: head.x--; break;
        case DIRECTION_RIGHT: head.x++; break;
    }
    return head;
}

/**
 * ACTION: convert relative action into an absolute direction
 * PARAMS: Direction, Action
 * RETURN: Direction
 */
static Direction get_new_direction(Direction current_dir, Action action) {
    if (action == ACTION_STRAIGHT) {
        return current_dir;
    }

    if (action == ACTION_LEFT) {
        switch (current_dir) {
            case DIRECTION_UP: return DIRECTION_LEFT;
            case DIRECTION_DOWN: return DIRECTION_RIGHT;
            case DIRECTION_LEFT: return DIRECTION_DOWN;
            case DIRECTION_RIGHT: return DIRECTION_UP;
        }
    } else {
        switch (current_dir) {
            case DIRECTION_UP: return DIRECTION_RIGHT;
            case DIRECTION_DOWN: return DIRECTION_LEFT;
            case DIRECTION_LEFT: return DIRECTION_UP;
            case DIRECTION_RIGHT: return DIRECTION_DOWN;
        }
    }
    return current_dir;
}

/**
 * ACTION: generate a new food position
 * PARAMS: Snake
 * RETURN: none
 */
static void spawn_food(Snake *game) {

    // if snake occupies full board then game is complete
    if(game->length >= MAX_SNAKE_LENGTH){
        game->done = true;
        return;
    }

    // else keep generating good somewhere at random
    Position food;
    do {
        food.x = rand() % GRID_WIDTH;
        food.y = rand() % GRID_HEIGHT;
    }
    while (snake_contains_position(game, food));
    game->food = food;
}

/**
 * ACTION: check whether a position collides with the snake.
 * PARAMS: Snake, Position, bool
 * RETURN: bool
 */
static bool collision_with_snake(const Snake *game, Position pos, bool ignore_tail) {
    int limit = game->length;

    // when snake moves without eating food, the tail will disappear during that same step.
    if (ignore_tail && limit > 0) limit--;
    for (int i = 0; i < limit; i++) {
        if (position_equal(game->snake[i], pos)) return true;
    }
    return false;
}

// MAIN PUBLIC APIS

void snake_seed(unsigned int seed) {
    srand(seed);
}

void snake_reset(Snake *game) {
    if (game==NULL) return;

    // initial snake lenght
    game->length = 2;

    // initial snake starting point
    int start_x = GRID_WIDTH / 2;
    int start_y = GRID_HEIGHT / 2;

    // initial snake movement direction
    game->direction = DIRECTION_RIGHT;

    // initial snake position
    game->snake[0].x = start_x;
    game->snake[0].y = start_y;
    game->snake[1].x = start_x-1;
    game->snake[1].y = start_y;

    // initial score
    game->score = 0;
    game->done = false;

    // start the loop
    spawn_food(game);
}

float snake_step(Snake *game, Action action) {
    if (game==NULL || game->done) return 0.0f;
    
    // check valid actions
    if (action<ACTION_STRAIGHT || action>ACTION_RIGHT) return 0.0f;
    
    // get new direction
    Direction new_direction=get_new_direction(game->direction, action);
    game->direction=new_direction;

    // get new head position
    Position new_head=get_next_position(game, game->direction);

    // check if out of grid
    if (!position_is_inside_grid(new_head)) {
        game->done=true;
        return WEIGHT_PUNISHMENT;
    }

    // check if food eaten
    bool food_eaten=position_equal(new_head, game->food);

    // check if collision with self
    if (collision_with_snake(game, new_head, !food_eaten)) {
        game->done=true;
        return WEIGHT_PUNISHMENT;
    }

    // increase if food eaten
    if (food_eaten){
        if(game->length<MAX_SNAKE_LENGTH) game->length++;
    }

    // update snake position
    for (int i=game->length-1; i > 0; i--) game->snake[i] = game->snake[i-1];
    
    game->snake[0]=new_head;

    // increase score
    if (food_eaten) {
        game->score++;
        spawn_food(game);
        return WEIGHT_FOOD;
    }

    return WEIGHT_SURVIVAL;
}

bool snake_is_done(const Snake *game) {
    if (game == NULL) return true;
    return game->done;
}

void snake_get_state(const Snake *game, int state[STATE_SIZE]) {
    if (game == NULL || state == NULL) return;
    Position head = game->snake[0];

    // determine the directions corresponding to the actions
    Direction straight = game->direction;
    Direction left = get_new_direction(game->direction, ACTION_LEFT);
    Direction right = get_new_direction(game->direction,ACTION_RIGHT);

    Position danger_straight = get_next_position(game, straight);
    Position danger_left = get_next_position(game, left);
    Position danger_right = get_next_position(game, right);

    // danger_right
    state[0] =
        !position_is_inside_grid(danger_straight) ||
        collision_with_snake(game, danger_straight, true);

    // danger_left
    state[1] =
        !position_is_inside_grid(danger_left) ||
        collision_with_snake(game, danger_left, true);

    // danger_right
    state[2] =
        !position_is_inside_grid(danger_right) ||
        collision_with_snake(game, danger_right, true);

    // food directions (relative to  global grid)
    state[3] = game->food.y < head.y;  // food_up
    state[4] = game->food.y > head.y;  // food_down
    state[5] = game->food.x < head.x;  // food_left
    state[6] = game->food.x > head.x;  // food_right

    // current direction
    state[7] = game->direction == DIRECTION_UP;
    state[8] = game->direction == DIRECTION_DOWN;
    state[9] = game->direction == DIRECTION_LEFT;
    state[10] = game->direction == DIRECTION_RIGHT;
}

void render_state(SDL_Renderer *renderer, const Snake *game, TTF_Font *font){
    if (renderer == NULL || game == NULL) return;

    // background
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // draw snake
    for (int i = 0; i < game->length; i++) {
        SDL_Rect segment;
        segment.x = game->snake[i].x * CELL_SIZE;
        segment.y = game->snake[i].y * CELL_SIZE;
        segment.w = CELL_SIZE;
        segment.h = CELL_SIZE;
        if (i == 0) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // snake head
        else SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);        // snake body
        SDL_RenderFillRect(renderer, &segment);
    }

    // draw food
    SDL_Rect food;
    food.x = game->food.x * CELL_SIZE;
    food.y = game->food.y * CELL_SIZE;
    food.w = CELL_SIZE;
    food.h = CELL_SIZE;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &food);

    // scoreboard
    char score_text[50];
    sprintf(score_text, "Score: %d", game->score);
    SDL_Color text_color = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font,score_text,text_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer,surface);
    SDL_Rect score_rect;
    score_rect.w = surface->w;
    score_rect.h = surface->h;
    score_rect.x = GRID_WIDTH * CELL_SIZE - score_rect.w - 10;
    score_rect.y = 10;
    SDL_RenderCopy(renderer,texture,NULL,&score_rect);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);

    // display frame
    SDL_RenderPresent(renderer);
}