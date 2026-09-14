#include "snake.h"

// HELPER FUNCTIONS

/**
 * ACTION: checks if two coordinates are equal
 * PARAMS: Position, Position
 * RETURN: bool
 */
static bool position_equal(Position pos1, Position pos2) {
    bool result = (pos1.x==pos2.y && pos1.y==pos2.y);
    return result;
}

/**
 * ACTION: check if a coordinate is inside allowed grid
 * PARAMS: Position
 * RETURN: bool
 */
static bool position_is_inside_grid(Position pos){
    bool result = (pos.x>=0 && pos.x<GRID_WIDTH && pos.y>=0 && pos.y<GRID_HEIGHT);
    return result;
}

/**
 * ACTION: check if snake exists in that coordinate
 * PARAMS: Snake, Position
 * RETURN: bool
 */
static bool snake_contains_position(const Snake *game, Position pos){
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
static Position get_next_position(const Snake *game, Direction direction){
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
static Direction get_new_direction(Direction current_dir, Action action){
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

// snake_reset
// snake_step

bool snake_is_done(const Snake *game) {
    if(game== NULL || game->done) return true;
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

// print_state