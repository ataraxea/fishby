#include <ncurses.h>
#include <stdlib.h>

int main(void)
{
    if (initscr() == NULL)
        return EXIT_FAILURE;

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    int rows, cols;
    int ch;

    do {
        getmaxyx(stdscr, rows, cols);

        erase();

        if (rows >= 5 && cols >= 20) {
            box(stdscr, 0, 0);

            mvaddstr(0, 2, " fishby ");
            mvaddstr(rows / 2, (cols - 7) / 2, "><(((o>");
            mvaddstr(rows - 2, 2, "[Q] Quit");
        } else {
            addnstr("Resize terminal", cols);
        }

        refresh();
        ch = getch();

    } while (ch != 'q' && ch != 'Q');

    endwin();

    return EXIT_SUCCESS;
}
