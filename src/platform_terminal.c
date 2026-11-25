#include <stddef.h>

#include "platform_terminal.h"

#ifdef _WIN32

static void terminal_sync_size(void)
{
    if (is_termresized()) {
        if (resize_term(0, 0) == OK)
            clearok(stdscr, TRUE);
    }
}

#endif

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

#ifdef _WIN32
    terminal_sync_size();
#endif

    return 0;
}

void terminal_shutdown(void)
{
    endwin();
}

void terminal_size(int *rows, int *cols)
{
#ifdef _WIN32
    terminal_sync_size();
#endif

    int height, width;

    getmaxyx(stdscr, height, width);

    *rows = height;
    *cols = width;
}

int terminal_read_key(void)
{
    int ch = getch();

#ifdef _WIN32
    if (ch == KEY_RESIZE) {
        if (resize_term(0, 0) == OK)
            clearok(stdscr, TRUE);
    }
#endif

    return ch;
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
