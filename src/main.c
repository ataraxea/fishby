#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "fish.h"
#include "bubbles.h"

#define TARGET_FPS 60L
#define FRAME_NS (1000000000L / TARGET_FPS)
#define MAX_DT 0.05f
#define BUBBLE_INTERVAL 0.65f

static float elapsed(struct timespec start,
                     struct timespec end)
{
    return (float)(end.tv_sec - start.tv_sec)
        + (float)(end.tv_nsec - start.tv_nsec) / 1e9f;
}

static int wait_next_frame(struct timespec start)
{
    start.tv_nsec += FRAME_NS;

    if (start.tv_nsec >= 1000000000L) {
        start.tv_sec++;
        start.tv_nsec -= 1000000000L;
    }

    int result;

    do {
        result = clock_nanosleep(
            CLOCK_MONOTONIC,
            TIMER_ABSTIME,
            &start,
            NULL
        );
    } while (result == EINTR);

    return result;
}

int main(void)
{
    if (initscr() == NULL)
        return EXIT_FAILURE;

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    Fish fish;
    fish_init(&fish);

    struct timespec previous;
    struct timespec current;

    if (clock_gettime(CLOCK_MONOTONIC, &previous) != 0) {
        endwin();
        return EXIT_FAILURE;
    }

    float bubble_timer = 0.0f;
    int running = 1;
    int exit_status = EXIT_SUCCESS;

    while (running) {
        if (clock_gettime(CLOCK_MONOTONIC, &current) != 0) {
            exit_status = EXIT_FAILURE;
            break;
        }

        float dt = elapsed(previous, current);
        previous = current;

        if (dt > MAX_DT)
            dt = MAX_DT;

        if (dt < 0.0f)
            dt = 0.0f;

        /* Process input */
        for (int i = 0; i < 32; i++) {
            int ch = getch();

            if (ch == ERR)
                break;

            if (ch == 'q' || ch == 'Q') {
                running = 0;
                break;
            }
        }

        if (!running)
            break;

        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        erase();

        bubbles_update(dt, rows, cols);

        if (rows >= 7 && cols >= 20) {

            /* Update fish physics */
            fish_update(&fish, dt, rows, cols);

            /* Spawn bubbles */
            bubble_timer += dt;

            if (bubble_timer >= BUBBLE_INTERVAL) {
                float mouth_x = fish.x;

                if (fish.direction > 0)
                    mouth_x += FISH_WIDTH - 1;

                bubbles_spawn(
                    mouth_x,
                    fish.y - 1.0f
                );

                bubble_timer -= BUBBLE_INTERVAL;
            }

            /* Draw aquarium */
            box(stdscr, 0, 0);

            mvaddstr(0, 2, " fishby ");

            for (int x = 1; x < cols - 1; x += 2)
                mvaddch(1, x, '~');

            /* Draw entities */
            bubbles_draw();
            fish_draw(&fish);

            /* Draw interface */
            mvaddstr(rows - 2, 2, "[Q] Quit");

        } else {
            addnstr("Resize terminal", cols);
        }

        /* Present frame */
        wnoutrefresh(stdscr);
        doupdate();

        /* Limit frame rate */
        if (wait_next_frame(current) != 0) {
            exit_status = EXIT_FAILURE;
            break;
        }
    }

    endwin();

    if (exit_status != EXIT_SUCCESS)
        fprintf(stderr, "fishby: animation timer failed\n");

    return exit_status;
}
