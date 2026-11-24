#include <ncurses.h>
#include <stdlib.h>
#include <math.h>

#include "food.h"

#define FOOD_SPEED 2.5f
#define FOOD_LIFETIME 30.0f
#define FOOD_PER_FEED 3

static Food particles[MAX_FOOD];
static int target_index = -1;

void food_spawn(int rows, int cols)
{
    if (rows < 7 || cols < 20)
        return;

    int spawned = 0;

    for (int i = 0; i < MAX_FOOD; i++) {
        if (particles[i].active)
            continue;

        particles[i] = (Food){
            .x = (float)(1 + rand() % (cols - 2)),
            .y = 2.0f,
            .age = 0.0f,
            .active = 1
        };

        if (++spawned >= FOOD_PER_FEED)
            break;
    }
}

void food_update(float dt, int rows, int cols)
{
    for (int i = 0; i < MAX_FOOD; i++) {
        Food *food = &particles[i];

        if (!food->active)
            continue;

        food->age += dt;

        if (rows < 7 || cols < 20 ||
            food->age >= FOOD_LIFETIME ||
            food->x < 1.0f ||
            food->x >= cols - 1) {
            food->active = 0;
            continue;
        }

        food->y += FOOD_SPEED * dt;

        float bottom = (float)(rows - 4);

        if (food->y > bottom)
            food->y = bottom;
    }
}

void food_draw(void)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    for (int i = 0; i < MAX_FOOD; i++) {
        const Food *food = &particles[i];

        if (!food->active)
            continue;

        int x = (int)roundf(food->x);
        int y = (int)roundf(food->y);

        if (x < 1 || x >= cols - 1 ||
            y < 2 || y >= rows - 3)
            continue;

        mvaddch(y, x, '*');
    }
}


int food_nearest(const Fish *fish, float *x, float *y)
{
    /* Keep chasing the current target */
    if (target_index >= 0 &&
        target_index < MAX_FOOD &&
        particles[target_index].active) {

        *x = particles[target_index].x;
        *y = particles[target_index].y;

        return 1;
    }

    target_index = -1;

    float best_distance = INFINITY;

    float center_x = fish->x
        + (FISH_WIDTH - 1) * 0.5f;

    for (int i = 0; i < MAX_FOOD; i++) {
        const Food *food = &particles[i];

        if (!food->active)
            continue;

        float dx = food->x - center_x;
        float dy = (food->y - fish->y) * 2.0f;

        float distance = dx * dx + dy * dy;

        if (distance < best_distance) {
            best_distance = distance;
            target_index = i;
        }
    }

    if (target_index < 0)
        return 0;

    *x = particles[target_index].x;
    *y = particles[target_index].y;

    return 1;
}



unsigned int food_eat(const Fish *fish)
{
    unsigned int eaten = 0;

    for (int i = 0; i < MAX_FOOD; i++) {
        Food *food = &particles[i];

        if (!food->active)
            continue;

        float left = fish->x - 1.0f;
        float right = fish->x + FISH_WIDTH;

        float dy = fabsf(food->y - fish->y);

        if (food->x >= left &&
            food->x <= right &&
            dy <= 1.0f) {

            food->active = 0;

            if (target_index == i)
                target_index = -1;

            eaten++;
        }
    }

    return eaten;
}

