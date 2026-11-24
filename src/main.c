#include <ncurses.h>
#include <stdlib.h>

#include "bubbles.h"

#define FISH_WIDTH 7
#define FRAME_DELAY 80
#define BUBBLE_INTERVAL 8

int main(void)
{
    if (initscr() == NULL)
        return EXIT_FAILURE;

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(FRAME_DELAY);

    int rows, cols;
    int fish_x = 1;
    int direction = 1;
    unsigned int frame = 0;
    int ch;

    do {
        getmaxyx(stdscr, rows, cols);
        erase();

        bubbles_update(rows, cols);

        if (rows >= 5 && cols >= 20) {
            int fish_y = rows / 2;
            int max_x = cols - FISH_WIDTH - 1;

            if (fish_x >= max_x) {
                fish_x = max_x;
                direction = -1;
            } else if (fish_x <= 1) {
                fish_x = 1;
                direction = 1;
            }

            box(stdscr, 0, 0);
            mvaddstr(0, 2, " fishby ");

            if (++frame % BUBBLE_INTERVAL == 0) {
                int mouth_x = direction > 0
                    ? fish_x + FISH_WIDTH - 1
                    : fish_x;

                bubbles_spawn(mouth_x, fish_y - 1);
            }

            bubbles_draw();

            mvaddstr(
                fish_y,
                fish_x,
                direction > 0 ? "><(((o>" : "<o)))><"
            );

            mvaddstr(rows - 2, 2, "[Q] Quit");

            fish_x += direction;
        } else {
            addnstr("Resize terminal", cols);
        }

        refresh();
        ch = getch();

    } while (ch != 'q' && ch != 'Q');

    endwin();

    return EXIT_SUCCESS;
}
