#ifndef SNAKE_H
#define SNAKE_H

#include<stdbool.h> 
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#define GRID_WIDTH 20
#define GRID_HEIGHT 20
#define MAX_SNAKE_LENGTH (GRID_HEIGHT * GRID_WIDTH)

typedef enum {
    ACTION_STRAIGHT = 0,
    ACTION_LEFT,
    ACTION_RIGHT
} Action;

typedef enum {
    DIRECTION_UP = 0,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT
} Direction;

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    int length;
    Position snake[MAX_SNAKE_LENGTH];
    Direction direction;
    Position food;
    int score;
    bool done;
} Snake;

/**
 * 01. danger_straight
 * 02. danger_left
 * 03. danger_right
 * 
 * 04. food_up
 * 05. food_down
 * 06. food_left
 * 07. food_right
 * 
 * 08. direction_up
 * 09. direction_down
 * 10. direction_left
 * 11. direction_right
 */
#define STATE_SIZE 11

/**
 * ACTION: create or reset the game state
 * PARAMS: Snake
 */
void snake_reset(Snake *game);

/**
 * ACTION: advance the game state by one step
 * PARAMS: Snake, Action
 * RETURN: Reward
 */
float snake_step(Snake *game, Action action);

/**
 * ACTION: check whether the episode has ended
 * PARAMS: Snake
 * RETURN: boolean
 */
bool snake_is_done(const Snake *game);

/**
 * ACTION: get current game state
 * PARAMS: Snake, state
 */
void snake_get_state(Snake *game, int state[STATE_SIZE]);

/**
 * ACTION: print current environment
 * PARAMS: Snake
 */
void print_state(Snake *game);

#endif