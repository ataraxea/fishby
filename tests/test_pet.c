#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>

#include "pet.h"

static void test_initial_state(void)
{
    Pet pet;
    pet_init(&pet);

    assert(pet.hunger == 30.0f);
    assert(pet.eaten == 0);
    assert(pet.mood == MOOD_CHILL);
}

static void test_hunger_growth(void)
{
    Pet pet;
    pet_init(&pet);

    float before = pet.hunger;

    pet_update(&pet, 0.05f);

    assert(pet.hunger > before);
}

static void test_feeding(void)
{
    Pet pet;
    pet_init(&pet);

    pet_feed(&pet, 1);

    assert(pet.eaten == 1);
    assert(fabsf(pet.hunger - 22.0f) < 0.001f);
    assert(pet.mood == MOOD_HAPPY);
}

static void test_hunger_limits(void)
{
    Pet pet;
    pet_init(&pet);

    pet.hunger = 99.99f;

    for (int i = 0; i < 100; i++)
        pet_update(&pet, 0.05f);

    assert(pet.hunger == 100.0f);

    pet_feed(&pet, 100);

    assert(pet.hunger == 0.0f);
    assert(pet.mood == MOOD_FULL);
}

static void test_mood_transitions(void)
{
    Pet pet;
    pet_init(&pet);

    pet.hunger = 60.0f;
    pet_update(&pet, 0.0f);

    assert(pet.mood == MOOD_HUNGRY);

    pet.hunger = 90.0f;
    pet_update(&pet, 0.0f);

    assert(pet.mood == MOOD_HANGRY);

    pet_feed(&pet, 1);

    assert(pet.mood == MOOD_HAPPY);
}

static void test_recovery(void)
{
    Pet pet;
    pet_init(&pet);

    pet_feed(&pet, 1);

    for (int i = 0; i < 200; i++)
        pet_update(&pet, 0.05f);

    assert(pet.happy_timer == 0.0f);
    assert(pet.mood == MOOD_CHILL);
}

static void test_counter_overflow(void)
{
    Pet pet;
    pet_init(&pet);

    pet.eaten = UINT64_MAX - 1;

    pet_feed(&pet, 3);

    assert(pet.eaten == UINT64_MAX);

    pet_feed(&pet, 1);

    assert(pet.eaten == UINT64_MAX);
}

static void test_empty_feed(void)
{
    Pet pet;
    pet_init(&pet);

    pet_feed(&pet, 0);

    assert(pet.hunger == 30.0f);
    assert(pet.eaten == 0);
    assert(pet.mood == MOOD_CHILL);
}

int main(void)
{
    test_initial_state();
    test_hunger_growth();
    test_feeding();
    test_hunger_limits();
    test_mood_transitions();
    test_recovery();
    test_counter_overflow();
    test_empty_feed();

    puts("PASS: pet unit tests (8/8)");

    return 0;
}
