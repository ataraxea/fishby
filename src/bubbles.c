#include <ncurses.h>
#include "bubbles.h"

static Bubble bubbles[MAX_BUBBLES];

void bubbles_spawn(int x, int y)
{
    for (int i = 0; i < MAX_BUBBLES; i++) {
        if (!bubbles[i].active) {
            bubbles[i] = (Bubble){
                .x = x,
                .y = y,
                .age = 0,
                .active = 1
            };

            return;
        }
    }
}

void bubbles_update(int rows, int cols)
{
    for (int i = 0; i < MAX_BUBBLES; i++) {
        Bubble *b = &bubbles[i];

        if (!b->active)
            continue;

        b->age++;

        if (b->age % 3 == 0)
            b->y--;

        if (b->y < 1 || b->y >= rows - 2 ||
            b->x < 1 || b->x >= cols - 1) {
            b->active = 0;
        }
    }
}

void bubbles_draw(void)
{
    for (int i = 0; i < MAX_BUBBLES; i++) {
        const Bubble *b = &bubbles[i];

        if (!b->active)
            continue;

        char sprite = '.';

        if (b->age >= 12)
            sprite = 'O';
        else if (b->age >= 6)
            sprite = 'o';

        mvaddch(b->y, b->x, sprite);
    }
}
