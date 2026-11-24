#ifndef FISHBY_FOOD_H
#define FISHBY_FOOD_H

#include "fish.h"

#define MAX_FOOD 64

typedef struct {
    float x;
    float y;
    float age;
    int active;
} Food;

void food_spawn(int rows, int cols);
void food_update(float dt, int rows, int cols);
void food_draw(void);

int food_nearest(const Fish *fish, float *x, float *y);
unsigned int food_eat(const Fish *fish);

#endif
