#include <math.h>
#include <limits.h>

#include "pet.h"

#define INITIAL_HUNGER 30.0f
#define HUNGER_RATE 0.20f
#define FOOD_NUTRITION 8.0f
#define HAPPY_DURATION 8.0f

static void pet_update_mood(Pet *pet)
{
    if (pet->hunger <= 5.0f)
        pet->mood = MOOD_FULL;
    else if (pet->happy_timer > 0.0f)
        pet->mood = MOOD_HAPPY;
    else if (pet->hunger >= 80.0f)
        pet->mood = MOOD_HANGRY;
    else if (pet->hunger >= 55.0f)
        pet->mood = MOOD_HUNGRY;
    else
        pet->mood = MOOD_CHILL;
}

void pet_init(Pet *pet)
{
    *pet = (Pet){
        .hunger = INITIAL_HUNGER,
        .happy_timer = 0.0f,
        .eaten = 0,
        .mood = MOOD_CHILL
    };
}

void pet_update(Pet *pet, float dt)
{
    dt = fmaxf(0.0f, fminf(dt, 0.05f));

    pet->hunger = fminf(
        100.0f,
        pet->hunger + HUNGER_RATE * dt
    );

    pet->happy_timer = fmaxf(
        0.0f,
        pet->happy_timer - dt
    );

    pet_update_mood(pet);
}

void pet_feed(Pet *pet, unsigned int amount)
{
    if (amount == 0)
        return;

    pet->hunger = fmaxf(
        0.0f,
        pet->hunger - FOOD_NUTRITION * amount
    );

    pet->happy_timer = HAPPY_DURATION;

    if (ULONG_MAX - pet->eaten >= amount)
        pet->eaten += amount;
    else
        pet->eaten = ULONG_MAX;

    pet_update_mood(pet);
}

const char *pet_mood_name(const Pet *pet)
{
    switch (pet->mood) {
        case MOOD_CHILL:
            return "chill";
        case MOOD_HAPPY:
            return "happy";
        case MOOD_HUNGRY:
            return "hungry";
        case MOOD_HANGRY:
            return "hangry";
        case MOOD_FULL:
            return "full";
        default:
            return "unknown";
    }
}

const char *pet_thought(const Pet *pet)
{
    switch (pet->mood) {
        case MOOD_CHILL:
            return "no thoughts. just blub.";
        case MOOD_HAPPY:
            return "nom nom nom.";
        case MOOD_HUNGRY:
            return "where food?";
        case MOOD_HANGRY:
            return "feed me, human.";
        case MOOD_FULL:
            return "food coma.";
        default:
            return "blub?";
    }
}
