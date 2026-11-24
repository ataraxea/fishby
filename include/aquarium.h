#ifndef FISHBY_AQUARIUM_H
#define FISHBY_AQUARIUM_H

typedef enum {
    AQUA_WATER = 1,
    AQUA_FISH,
    AQUA_BUBBLE,
    AQUA_PLANT,
    AQUA_SAND,
    AQUA_BORDER
} AquariumColor;

void aquarium_init(void);
void aquarium_draw(float dt, int rows, int cols);

void aquarium_color_on(AquariumColor color);
void aquarium_color_off(AquariumColor color);

#endif
