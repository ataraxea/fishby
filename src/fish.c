#include "platform_terminal.h"
#include <math.h>

#include "fish.h"

#define TAU 6.283185307f

#define SWIM_SPEED 10.0f
#define CHASE_SPEED 24.0f
#define CHASE_VERTICAL_SPEED 12.0f
#define RECOVERY_TIME 0.25f

static float clampf(float value, float low, float high)
{
    return fmaxf(low, fminf(value, high));
}

void fish_init(Fish *fish)
{
    *fish = (Fish){
        .direction = 1,
        .heading = 1
    };
}

void fish_on_eat(Fish *fish)
{
    fish->recovery_timer = RECOVERY_TIME;
    fish->heading = fish->direction;
}

void fish_update(
    Fish *fish,
    float dt,
    int rows,
    int cols,
    int has_target,
    float target_x,
    float target_y
)
{
    if (rows < 7 || cols < 20)
        return;

    dt = clampf(dt, 0.0f, 0.05f);

    const float left = 1.0f;
    const float right = (float)(cols - FISH_WIDTH - 1);
    const float top = 2.0f;
    const float bottom = (float)(rows - 3);

    const float middle = (top + bottom) * 0.5f;
    const float amplitude = (bottom - top) * 0.40f;

    if (!fish->initialized) {
        fish->x = left + (right - left) * 0.25f;
        fish->y = middle;
        fish->initialized = 1;
    }

    fish->x = clampf(fish->x, left, right);
    fish->y = clampf(fish->y, top, bottom);

    fish->phase = fmodf(
        fish->phase + dt * 0.55f,
        TAU * 4.0f
    );

    float target_vx = 0.0f;
    float target_vy = 0.0f;
    float responsiveness = 3.5f;

    /* Recover after eating */
    if (fish->recovery_timer > 0.0f) {

        fish->recovery_timer = fmaxf(
            0.0f, fish->recovery_timer - dt
        );

        target_vx = fish->direction * 6.0f;
        target_vy = 0.0f;

        responsiveness = 5.0f;

    /* Chase food */
    } else if (has_target) {

        float center_x = fish->x
            + (FISH_WIDTH - 1) * 0.5f;

        float dx = target_x - center_x;
        float dy = target_y - fish->y;

        target_vx = clampf(
            dx * 5.0f,
            -CHASE_SPEED,
            CHASE_SPEED
        );

        target_vy = clampf(
            dy * 5.0f,
            -CHASE_VERTICAL_SPEED,
            CHASE_VERTICAL_SPEED
        );

        responsiveness = 7.0f;

    /* Natural swimming */
    } else {

        float speed = SWIM_SPEED
            + 2.5f * sinf(fish->phase * 0.75f);

        float margin = fminf(
            5.0f, (right - left) * 0.25f
        );

        if (fish->heading > 0 &&
            fish->x >= right - margin) {
            fish->heading = -1;
        } else if (fish->heading < 0 &&
                   fish->x <= left + margin) {
            fish->heading = 1;
        }

        target_vx = speed * fish->heading;

        float desired_y = middle + amplitude * (
            0.75f * sinf(fish->phase)
            + 0.25f * sinf(fish->phase * 1.75f)
        );

        target_vy = clampf(
            (desired_y - fish->y) * 1.8f,
            -5.0f,
            5.0f
        );
    }

    /* Smooth velocity transitions */
    float blend = 1.0f - expf(
        -responsiveness * dt
    );

    fish->vx += (target_vx - fish->vx) * blend;
    fish->vy += (target_vy - fish->vy) * blend;

    fish->x = clampf(
        fish->x + fish->vx * dt, left, right
    );

    fish->y = clampf(
        fish->y + fish->vy * dt, top, bottom
    );

    /* Prevent velocity buildup against walls */
    if ((fish->x <= left && fish->vx < 0.0f) ||
        (fish->x >= right && fish->vx > 0.0f)) {
        fish->vx = 0.0f;
    }

    if ((fish->y <= top && fish->vy < 0.0f) ||
        (fish->y >= bottom && fish->vy > 0.0f)) {
        fish->vy = 0.0f;
    }

    /* Update facing direction */
    if (fish->vx > 0.6f)
        fish->direction = 1;
    else if (fish->vx < -0.6f)
        fish->direction = -1;

    /* Animate tail */
    fish->tail_phase = fmodf(
        fish->tail_phase
        + dt * (4.0f + fabsf(fish->vx) * 0.35f),
        TAU
    );

    /* Animate eyes */
    fish->blink_timer = fmodf(
        fish->blink_timer + dt, 7.0f
    );
}

void fish_draw(const Fish *fish)
{
    char sprite[] = "><(((o>";

    int tail = sinf(fish->tail_phase) > 0.0f;
    int blink = fish->blink_timer > 6.85f;
    int eating = fish->recovery_timer > 0.0f;

    if (fish->direction > 0) {
        sprite[0] = tail ? '>' : '}';
        sprite[5] = blink ? '-' : 'o';
        sprite[6] = eating ? ')' : '>';
    } else {
        sprite[0] = eating ? '(' : '<';
        sprite[1] = blink ? '-' : 'o';
        sprite[2] = ')';
        sprite[3] = ')';
        sprite[4] = ')';
        sprite[5] = '>';
        sprite[6] = tail ? '<' : '{';
    }

    mvaddstr(
        (int)roundf(fish->y),
        (int)roundf(fish->x),
        sprite
    );
}
