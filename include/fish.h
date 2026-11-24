#ifndef FISHBY_FISH_H
#define FISHBY_FISH_H

#define FISH_WIDTH 7

typedef struct {
    float x, y;
    float vx, vy;

    float phase;
    float tail_phase;
    float blink_timer;

    int direction;
    int heading;
    int initialized;
} Fish;

void fish_init(Fish *fish);
void fish_update(Fish *fish, float dt, int rows, int cols);
void fish_draw(const Fish *fish);

#endif
