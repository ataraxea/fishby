#ifndef FISHBY_BUBBLES_H
#define FISHBY_BUBBLES_H

#define MAX_BUBBLES 32

typedef struct {
    float x;
    float y;
    float age;
    int active;
} Bubble;

void bubbles_spawn(float x, float y);
void bubbles_update(float dt, int rows, int cols);
void bubbles_draw(void);

#endif
