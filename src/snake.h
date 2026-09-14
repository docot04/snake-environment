#ifndef SNAKE_H
#define SNAKE_H

typedef struct {

} Game;

void reset(Game *game);
void step(Game *game, int action);

void get_state(Game *game, float *state);
float get_reward(Game *game);
int is_done(Game *game);

#endif