#ifndef FISHBY_PLATFORM_TERMINAL_H
#define FISHBY_PLATFORM_TERMINAL_H

#ifdef _WIN32
#include <curses.h>
#else
#include <ncurses.h>
#endif

int terminal_init(void);
void terminal_shutdown(void);

void terminal_size(int *rows, int *cols);

int terminal_read_key(void);
int terminal_present(void);

void terminal_clear(void);

#endif
