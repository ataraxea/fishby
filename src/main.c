#include <ncurses.h>
#include <stdlib.h>

#define FISH_WIDTH 7
#define FRAME_DELAY 80

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
    int ch;

    do {
        getmaxyx(stdscr, rows, cols);
        erase();

        if (rows >= 5 && cols >= 20) {
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

            mvaddstr(
                rows / 2,
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
