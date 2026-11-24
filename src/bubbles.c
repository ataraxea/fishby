#include <ncurses.h>
#include <math.h>

#include "bubbles.h"

#define BUBBLE_SPEED 3.0f
#define BUBBLE_DRIFT 0.8f
#define BUBBLE_FREQUENCY 2.5f

static Bubble bubbles[MAX_BUBBLES];

static int bubble_x(const Bubble *b)
{
    return (int)roundf(
        b->x + sinf(b->age * BUBBLE_FREQUENCY)
        * BUBBLE_DRIFT
    );
}

void bubbles_spawn(float x, float y)
{
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (!bubbles[i].active) {
            bubbles[i] = (Bubble){
                .x = x,
                .y = y,
                .age = 0.0f,
                .active = 1
            };

            return;
        }
    }
}

void bubbles_update(float dt, int rows, int cols)
{
    for (int i = 0; i < MAX_BUBBLES; i++) {
        Bubble *b = &bubbles[i];

        if (!b->active)
            continue;

        b->age += dt;
        b->y -= BUBBLE_SPEED * dt;

        int x = bubble_x(b);
        int y = (int)roundf(b->y);

        if (x < 1 || x >= cols - 1 ||
            y < 2 || y >= rows - 2) {
            b->active = 0;
        }
    }
}

void bubbles_draw(void)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    for (int i = 0; i < MAX_BUBBLES; i++) {
        const Bubble *b = &bubbles[i];

        if (!b->active)
            continue;

        int x = bubble_x(b);
        int y = (int)roundf(b->y);

        if (x < 1 || x >= cols - 1 ||
            y < 2 || y >= rows - 2)
            continue;

        char sprite = '.';

        if (b->age >= 1.2f)
            sprite = 'O';
        else if (b->age >= 0.5f)
            sprite = 'o';

        mvaddch(y, x, sprite);
    }
}
