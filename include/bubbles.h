#ifndef FISHBY_BUBBLES_H
#define FISHBY_BUBBLES_H

#define MAX_BUBBLES 32

typedef struct {
    int x;
    int y;
    unsigned int age;
    int active;
} Bubble;

void bubbles_spawn(int x, int y);
void bubbles_update(int rows, int cols);
void bubbles_draw(void);

#endif
