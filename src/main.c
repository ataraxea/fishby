#define _POSIX_C_SOURCE 200809L

#include "platform_terminal.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "fish.h"
#include "bubbles.h"
#include "aquarium.h"
#include "food.h"
#include "pet.h"
#include "state.h"
#include "cli.h"
#include "signals.h"
#include "platform_time.h"

#define SIMULATION_HZ 60.0
#define SIMULATION_DT (1.0 / SIMULATION_HZ)

#define MAX_FRAME_TIME 0.25
#define MAX_SIMULATION_STEPS 16

#define BUBBLE_INTERVAL 0.65f
#define FEED_COOLDOWN 0.3f
#define SAVE_INTERVAL 30.0
#define FPS_SAMPLE_INTERVAL 1.0

typedef struct {
    Fish fish;
    Pet pet;

    float bubble_timer;
    float feed_cooldown;
} Simulation;

typedef struct {
    double elapsed;
    unsigned int frames;
    double measured;
} FpsCounter;

static int wait_next_frame(
    double start,
    double frame_duration
)
{
    double deadline = start + frame_duration;

    while (!signals_requested()) {
        int result = platform_sleep_until(deadline);

        if (result <= 0)
            return result;
    }

    return 0;
}

static void fps_update(
    FpsCounter *counter,
    double dt
)
{
    counter->elapsed += dt;

    if (counter->elapsed >= FPS_SAMPLE_INTERVAL) {
        counter->measured =
            (double)counter->frames /
            counter->elapsed;

        counter->frames = 0;
        counter->elapsed = 0.0;
    }
}

static void simulation_init(Simulation *sim)
{
    *sim = (Simulation){0};

    fish_init(&sim->fish);
    pet_init(&sim->pet);
}

static void simulation_update(
    Simulation *sim,
    float dt,
    int rows,
    int cols
)
{
    if (sim->feed_cooldown > 0.0f)
        sim->feed_cooldown -= dt;

    pet_update(&sim->pet, dt);
    bubbles_update(dt, rows, cols);
    food_update(dt, rows, cols);

    if (rows < 7 || cols < 20)
        return;

    float target_x = 0.0f;
    float target_y = 0.0f;

    int has_target = food_nearest(
        &sim->fish,
        &target_x,
        &target_y
    );

    fish_update(
        &sim->fish,
        dt,
        rows,
        cols,
        has_target,
        target_x,
        target_y
    );

    /* Consume food */
    unsigned int consumed = 0;

    if (sim->fish.recovery_timer <= 0.0f)
        consumed = food_eat(&sim->fish);

    if (consumed > 0) {
        fish_on_eat(&sim->fish);
        pet_feed(&sim->pet, consumed);
    }

    /* Spawn bubbles at a fixed simulation rate */
    sim->bubble_timer += dt;

    if (sim->bubble_timer >= BUBBLE_INTERVAL) {
        float mouth_x = sim->fish.x;

        if (sim->fish.direction > 0)
            mouth_x += FISH_WIDTH - 1;

        bubbles_spawn(
            mouth_x,
            sim->fish.y - 1.0f
        );

        sim->bubble_timer -= BUBBLE_INTERVAL;
    }
}

static void draw_status(
    const Pet *pet,
    const CliOptions *options,
    const FpsCounter *fps,
    int rows,
    int cols,
    int save_error
)
{
    char status[192];

    if (cols >= 90) {
        snprintf(
            status,
            sizeof(status),
            "[F] Feed | [Q] Quit | Hunger: %d%%"
            " | Mood: %s | Eaten: %lu"
            " | FPS: %.0f/%d",
            (int)pet->hunger,
            pet_mood_name(pet),
            pet->eaten,
            fps->measured,
            options->fps
        );
    } else {
        snprintf(
            status,
            sizeof(status),
            "FPS: %.0f/%d | F/Q | H:%d%%"
            " | %s | E:%lu",
            fps->measured,
            options->fps,
            (int)pet->hunger,
            pet_mood_name(pet),
            pet->eaten
        );
    }

    mvaddnstr(rows - 2, 2, status, cols - 3);

    /* Draw aquarium title */
    char title[64];

    if (strcmp(options->name, "fishby") == 0) {
        snprintf(
            title,
            sizeof(title),
            " fishby "
        );
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

static void render_frame(
    const Simulation *sim,
    const CliOptions *options,
    const FpsCounter *fps,
    float visual_dt,
    int rows,
    int cols,
    int save_error
)
{
    terminal_clear();

    if (rows < 7 || cols < 20) {
        addnstr("Resize terminal", cols);
        return;
    }

    aquarium_draw(visual_dt, rows, cols);

    aquarium_color_on(AQUA_BUBBLE);
    bubbles_draw();
    aquarium_color_off(AQUA_BUBBLE);

    aquarium_color_on(AQUA_SAND);
    food_draw();
    aquarium_color_off(AQUA_SAND);

    aquarium_color_on(AQUA_FISH);
    fish_draw(&sim->fish);
    aquarium_color_off(AQUA_FISH);

    draw_status(
        &sim->pet,
        options,
        fps,
        rows,
        cols,
        save_error
    );
}

static int process_input(
    Simulation *sim,
    int rows,
    int cols
)
{
    for (int i = 0; i < 32; i++) {
        if (signals_requested())
            return 0;

        int ch = terminal_read_key();

        if (ch == ERR)
            break;

        if (ch == 'q' || ch == 'Q')
            return 0;

        if ((ch == 'f' || ch == 'F') &&
            sim->feed_cooldown <= 0.0f &&
            rows >= 7 && cols >= 20) {

            food_spawn(rows, cols);
            sim->feed_cooldown = FEED_COOLDOWN;
        }
    }

    return 1;
}

int main(int argc, char **argv)
{
    CliOptions options;

    CliResult result = cli_parse(
        &options,
        argc,
        argv
    );

    if (result == CLI_EXIT)
        return EXIT_SUCCESS;

    if (result == CLI_ERROR)
        return EXIT_FAILURE;

    const double frame_duration =
        1.0 / options.fps;

    Simulation sim;
    simulation_init(&sim);

    FpsCounter fps = {0};

    int state_dir = -1;
    int save_error = 0;

    /* Initialize persistent storage */
    if (!options.no_save) {
        state_dir = state_open();

        if (state_dir < 0) {
            save_error = 1;

        } else if (state_load(
                       state_dir,
                       &sim.pet
                   ) < 0) {

            close(state_dir);
            state_dir = -1;
            save_error = 1;
        }
    }

    if (terminal_init() < 0) {
        if (state_dir >= 0)
            close(state_dir);

        return EXIT_FAILURE;
    }

    /* Install handlers after ncurses initialization */
    if (signals_init() < 0) {
        terminal_shutdown();

        if (state_dir >= 0)
            close(state_dir);

        fprintf(
            stderr,
            "fishby: signal initialization failed\n"
        );

        return EXIT_FAILURE;
    }

    if (!options.no_color)
        aquarium_init();

    srand((unsigned int)time(NULL));

    double previous;
    double current;

    if (platform_time_now(&previous) != 0) {

        terminal_shutdown();

        if (state_dir >= 0)
            close(state_dir);

        return EXIT_FAILURE;
    }

    double accumulator = 0.0;
    double save_timer = 0.0;

    int exit_status = EXIT_SUCCESS;

    while (!signals_requested()) {

        if (platform_time_now(&current) != 0) {

            exit_status = EXIT_FAILURE;
            break;
        }

        double real_dt = current - previous;

        previous = current;

        if (real_dt < 0.0)
            real_dt = 0.0;

        /* Measure actual rendered frames */
        fps_update(&fps, real_dt);

        /* Autosave uses real elapsed time */
        save_timer += real_dt;

        /* Bound simulation catch-up time */
        double frame_dt = real_dt;

        if (frame_dt > MAX_FRAME_TIME)
            frame_dt = MAX_FRAME_TIME;

        accumulator += frame_dt;

        int rows, cols;
        terminal_size(&rows, &cols);

        if (!process_input(&sim, rows, cols))
            break;

        terminal_size(&rows, &cols);

        /* Fixed-step simulation */
        int steps = 0;
        double simulated_time = 0.0;

        while (accumulator >= SIMULATION_DT &&
               steps < MAX_SIMULATION_STEPS &&
               !signals_requested()) {

            simulation_update(
                &sim,
                (float)SIMULATION_DT,
                rows,
                cols
            );

            accumulator -= SIMULATION_DT;
            simulated_time += SIMULATION_DT;

            steps++;
        }

        /* Prevent unbounded simulation backlog */
        if (accumulator >= SIMULATION_DT)
            accumulator = 0.0;

        if (signals_requested())
            break;

        /* Render once per outer loop */
        render_frame(
            &sim,
            &options,
            &fps,
            (float)simulated_time,
            rows,
            cols,
            save_error
        );

        /* Present the completed frame */
        if (terminal_present() == 0) {

            fps.frames++;
        }

        /* Periodic autosave */
        if (state_dir >= 0 &&
            save_timer >= SAVE_INTERVAL) {

            save_error = state_save(
                state_dir,
                &sim.pet
            ) < 0;

            save_timer = 0.0;
        }

        /* Limit rendering frame rate */
        if (wait_next_frame(
                current,
                frame_duration
            ) != 0) {

            exit_status = EXIT_FAILURE;
            break;
        }
    }

    /* Restore terminal state */
    terminal_shutdown();

    /* Save pet state before exiting */
    if (state_dir >= 0) {
        save_error = state_save(
            state_dir,
            &sim.pet
        ) < 0;

        close(state_dir);
    }

    if (save_error)
        fprintf(
            stderr,
            "fishby: state unavailable\n"
        );

    if (exit_status != EXIT_SUCCESS)
        fprintf(
            stderr,
            "fishby: animation timer failed\n"
        );

    int signo = signals_received();

    if (signo != 0)
        return 128 + signo;

    return exit_status;
}
