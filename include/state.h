#ifndef FISHBY_STATE_H
#define FISHBY_STATE_H

#include "pet.h"

int state_open(void);
int state_load(int dir, Pet *pet);
int state_save(int dir, const Pet *pet);

void state_close(int dir);

#endif
