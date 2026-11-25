#include <stddef.h>

#include "platform_terminal.h"

int terminal_init(void)
{
    if (initscr() == NULL)
        return -1;

    if (cbreak() == ERR ||
        noecho() == ERR ||
        keypad(stdscr, TRUE) == ERR ||
        nodelay(stdscr, TRUE) == ERR) {

        endwin();
        return -1;
    }

    /* Cursor visibility is not supported everywhere */
    curs_set(0);

    return 0;
}

void terminal_shutdown(void)
{
    endwin();
}

void terminal_size(int *rows, int *cols)
{
    int height, width;

    getmaxyx(stdscr, height, width);

    *rows = height;
    *cols = width;
}

int terminal_read_key(void)
{
    return getch();
}

int terminal_present(void)
{
    if (wnoutrefresh(stdscr) == ERR)
        return -1;

    if (doupdate() == ERR)
        return -1;

    return 0;
}

void terminal_clear(void)
{
    erase();
}
