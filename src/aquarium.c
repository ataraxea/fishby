#include "platform_terminal.h"
#include <math.h>

#include "aquarium.h"

#define TAU 6.283185307f

static int colors_enabled = 0;
static float phase = 0.0f;

void aquarium_init(void)
{
    if (!has_colors())
        return;

    if (start_color() == ERR)
        return;

    if (COLORS < 8 || COLOR_PAIRS <= AQUA_BORDER)
        return;

    short background = COLOR_BLACK;

    if (use_default_colors() == OK)
        background = -1;

    if (init_pair(AQUA_WATER, COLOR_BLUE, background) == ERR ||
        init_pair(AQUA_FISH, COLOR_YELLOW, background) == ERR ||
        init_pair(AQUA_BUBBLE, COLOR_CYAN, background) == ERR ||
        init_pair(AQUA_PLANT, COLOR_GREEN, background) == ERR ||
        init_pair(AQUA_SAND, COLOR_YELLOW, background) == ERR ||
        init_pair(AQUA_BORDER, COLOR_WHITE, background) == ERR)
        return;

    colors_enabled = 1;
}

void aquarium_color_on(AquariumColor color)
{
    if (colors_enabled)
        attron(COLOR_PAIR(color));
}

void aquarium_color_off(AquariumColor color)
{
    if (colors_enabled)
        attroff(COLOR_PAIR(color));
}

static void draw_water(int cols)
{
    aquarium_color_on(AQUA_WATER);

    for (int x = 1; x < cols - 1; x++) {
        float wave = sinf(x * 0.45f - phase);

        char sprite = wave > 0.15f ? '~' : '-';

        mvaddch(1, x, sprite);
    }

    aquarium_color_off(AQUA_WATER);
}

static void draw_sand(int rows, int cols)
{
    aquarium_color_on(AQUA_SAND);

    for (int x = 1; x < cols - 1; x++) {
        char sprite = '.';

        if (x % 17 == 0)
            sprite = 'o';
        else if (x % 7 == 0)
            sprite = ',';
        else if (x % 5 == 0)
            sprite = ' ';

        mvaddch(rows - 3, x, sprite);
    }

    aquarium_color_off(AQUA_SAND);
}

static void draw_plants(int rows, int cols)
{
    int height = rows / 5;

    if (height > 6)
        height = 6;

    aquarium_color_on(AQUA_PLANT);

    for (int base = 4; base < cols - 3; base += 14) {
        for (int i = 0; i < height; i++) {
            float sway = sinf(
                phase + base * 0.3f + i * 0.45f
            );

            int offset = (int)roundf(
                sway * i * 0.3f
            );

            int x = base + offset;
            int y = rows - 4 - i;

            if (x < 1 || x >= cols - 1 || y < 2)
                continue;

            char sprite = '|';

            if (sway > 0.25f)
                sprite = ')';
            else if (sway < -0.25f)
                sprite = '(';

            mvaddch(y, x, sprite);
        }
    }

    aquarium_color_off(AQUA_PLANT);
}

void aquarium_draw(float dt, int rows, int cols)
{
    if (rows < 7 || cols < 20)
        return;

    phase = fmodf(phase + dt * 1.5f, TAU);

    aquarium_color_on(AQUA_BORDER);

    box(stdscr, 0, 0);
    mvaddstr(0, 2, " fishby ");

    aquarium_color_off(AQUA_BORDER);

    draw_water(cols);
    draw_sand(rows, cols);
    draw_plants(rows, cols);
}
