#define _POSIX_C_SOURCE 200809L

#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "fish.h"
#include "bubbles.h"
#include "aquarium.h"
#include "food.h"
#include "pet.h"
#include "state.h"
#include "cli.h"

#define MAX_DT 0.05f
#define BUBBLE_INTERVAL 0.65f
#define FEED_COOLDOWN 0.3f
#define SAVE_INTERVAL 30.0f

static float elapsed(
    struct timespec start,
    struct timespec end
)
{
    return (float)(end.tv_sec - start.tv_sec)
        + (float)(end.tv_nsec - start.tv_nsec) / 1e9f;
}

static int wait_next_frame(
    struct timespec start,
    long frame_ns
)
{
    start.tv_nsec += frame_ns;

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

static void draw_status(
    const Pet *pet,
    const CliOptions *options,
    int rows,
    int cols,
    int save_error
)
{
    char status[192];

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

    mvaddnstr(rows - 2, 2, status, cols - 3);

    /* Draw the title and fish name */
    char title[64];

    if (strcmp(options->name, "fishby") == 0) {
        snprintf(title, sizeof(title), " fishby ");
    } else {
        snprintf(
            title,
            sizeof(title),
            " fishby / %s ",
            options->name
        );
    }

    aquarium_color_on(AQUA_BORDER);

    mvaddnstr(0, 2, title, cols - 3);

    aquarium_color_off(AQUA_BORDER);

    /* Draw fish thoughts */
    int thought_x = 3 + (int)strlen(title);

    if (cols - thought_x >= 20) {
        char thought[64];

        const char *message = pet_thought(pet);

        if (save_error)
            message = "save unavailable";
        else if (options->no_save)
            message = "session only";

        snprintf(
            thought,
            sizeof(thought),
            " %s ",
            message
        );

        aquarium_color_on(AQUA_FISH);

        mvaddnstr(
            0,
            thought_x,
            thought,
            cols - thought_x - 1
        );

        aquarium_color_off(AQUA_FISH);
    }
}

int main(int argc, char **argv)
{
    CliOptions options;

    CliResult result = cli_parse(
        &options, argc, argv
    );

    if (result == CLI_EXIT)
        return EXIT_SUCCESS;

    if (result == CLI_ERROR)
        return EXIT_FAILURE;

    const long frame_ns =
        1000000000L / options.fps;

    Fish fish;
    fish_init(&fish);

    Pet pet;
    pet_init(&pet);

    /* Initialize persistent storage */
    int state_dir = -1;
    int save_error = 0;

    if (!options.no_save) {
        state_dir = state_open();

        if (state_dir < 0) {
            save_error = 1;
        } else if (state_load(state_dir, &pet) < 0) {
            close(state_dir);
            state_dir = -1;
            save_error = 1;
        }
    }

    if (initscr() == NULL) {
        if (state_dir >= 0)
            close(state_dir);

        return EXIT_FAILURE;
    }

    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    if (!options.no_color)
        aquarium_init();

    srand((unsigned int)time(NULL));

    struct timespec previous;
    struct timespec current;

    if (clock_gettime(CLOCK_MONOTONIC, &previous) != 0) {
        endwin();

        if (state_dir >= 0)
            close(state_dir);

        return EXIT_FAILURE;
    }

    float bubble_timer = 0.0f;
    float feed_cooldown = 0.0f;
    float save_timer = 0.0f;

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

        /* Process input */
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

            /* Consume food */
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

            draw_status(
                &pet,
                &options,
                rows,
                cols,
                save_error
            );

        } else {
            addnstr("Resize terminal", cols);
        }

        /* Present frame */
        wnoutrefresh(stdscr);
        doupdate();

        /* Periodic autosave */
        if (state_dir >= 0) {
            save_timer += dt;

            if (save_timer >= SAVE_INTERVAL) {
                save_error = state_save(
                    state_dir, &pet
                ) < 0;

                save_timer = 0.0f;
            }
        }

        /* Limit frame rate */
        if (wait_next_frame(current, frame_ns) != 0) {
            exit_status = EXIT_FAILURE;
            break;
        }
    }

    endwin();

    /* Save state before exiting */
    if (state_dir >= 0) {
        save_error = state_save(
            state_dir, &pet
        ) < 0;

        close(state_dir);
    }

    if (save_error)
        fprintf(stderr, "fishby: state unavailable\n");

    if (exit_status != EXIT_SUCCESS)
        fprintf(stderr, "fishby: animation timer failed\n");

    return exit_status;
}
