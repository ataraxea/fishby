#ifndef FISHBY_PET_H
#define FISHBY_PET_H

typedef enum {
    MOOD_CHILL,
    MOOD_HAPPY,
    MOOD_HUNGRY,
    MOOD_HANGRY,
    MOOD_FULL
} PetMood;

typedef struct {
    float hunger;
    float happy_timer;
    unsigned long eaten;
    PetMood mood;
} Pet;

void pet_init(Pet *pet);
void pet_update(Pet *pet, float dt);
void pet_feed(Pet *pet, unsigned int amount);

const char *pet_mood_name(const Pet *pet);
const char *pet_thought(const Pet *pet);

#endif
