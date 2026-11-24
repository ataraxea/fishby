#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "fish.h"
#include "bubbles.h"
#include "aquarium.h"
#include "food.h"
#include "pet.h"

#define TARGET_FPS 60L
#define FRAME_NS (1000000000L / TARGET_FPS)
#define MAX_DT 0.05f
#define BUBBLE_INTERVAL 0.65f
#define FEED_COOLDOWN 0.3f

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

static void draw_status(const Pet *pet,
                        int rows, int cols)
{
    char status[128];

    if (cols >= 65) {
        snprintf(
            status,
            sizeof(status),
            "[F] Feed | [Q] Quit | Hunger: %d%%"
            " | Mood: %s | Eaten: %lu",
            (int)pet->hunger,
            pet_mood_name(pet),
            pet->eaten
        );
    } else {
        snprintf(
            status,
            sizeof(status),
            "F/Q | H:%d%% | %s | E:%lu",
            (int)pet->hunger,
            pet_mood_name(pet),
            pet->eaten
        );
    }

    mvaddnstr(
        rows - 2,
        2,
        status,
        cols - 3
    );

    /* Display fish thoughts in the top border */
    if (cols >= 40) {
        char thought[64];

        snprintf(
            thought,
            sizeof(thought),
            " %s ",
            pet_thought(pet)
        );

        aquarium_color_on(AQUA_FISH);

        mvaddnstr(
            0,
            12,
            thought,
            cols - 13
        );

        aquarium_color_off(AQUA_FISH);
    }
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

    aquarium_init();

    srand((unsigned int)time(NULL));

    Fish fish;
    fish_init(&fish);

    Pet pet;
    pet_init(&pet);

    struct timespec previous;
    struct timespec current;

    if (clock_gettime(CLOCK_MONOTONIC, &previous) != 0) {
        endwin();
        return EXIT_FAILURE;
    }

    float bubble_timer = 0.0f;
    float feed_cooldown = 0.0f;

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

        int rows, cols;
        getmaxyx(stdscr, rows, cols);

        if (feed_cooldown > 0.0f)
            feed_cooldown -= dt;

        /* Process keyboard input */
        for (int i = 0; i < 32; i++) {
            int ch = getch();

            if (ch == ERR)
                break;

            if (ch == 'q' || ch == 'Q') {
                running = 0;
                break;
            }

            if ((ch == 'f' || ch == 'F') &&
                feed_cooldown <= 0.0f &&
                rows >= 7 && cols >= 20) {

                food_spawn(rows, cols);
                feed_cooldown = FEED_COOLDOWN;
            }
        }

        if (!running)
            break;

        getmaxyx(stdscr, rows, cols);

        erase();

        /* Update pet state and particles */
        pet_update(&pet, dt);
        bubbles_update(dt, rows, cols);
        food_update(dt, rows, cols);

        if (rows >= 7 && cols >= 20) {

            float target_x = 0.0f;
            float target_y = 0.0f;

            int has_target = food_nearest(
                &fish,
                &target_x,
                &target_y
            );

            /* Update fish movement */
            fish_update(
                &fish,
                dt,
                rows,
                cols,
                has_target,
                target_x,
                target_y
            );

            /* Consume food and update pet state */
            unsigned int consumed = 0;

            if (fish.recovery_timer <= 0.0f)
                consumed = food_eat(&fish);

            if (consumed > 0) {
                fish_on_eat(&fish);
                pet_feed(&pet, consumed);
            }

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
            aquarium_draw(dt, rows, cols);

            aquarium_color_on(AQUA_BUBBLE);
            bubbles_draw();
            aquarium_color_off(AQUA_BUBBLE);

            aquarium_color_on(AQUA_SAND);
            food_draw();
            aquarium_color_off(AQUA_SAND);

            aquarium_color_on(AQUA_FISH);
            fish_draw(&fish);
            aquarium_color_off(AQUA_FISH);

            draw_status(&pet, rows, cols);

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
