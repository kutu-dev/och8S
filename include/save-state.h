#ifndef OCH8S_SAVE_STATES_H
#define OCH8S_SAVE_STATES_H

#include <stdint.h>

#include "render.h"
#include "virtual-machine.h"

uint8_t save_state(struct VirtualMachine* vm, struct Screen* screen);

uint8_t load_state(struct VirtualMachine* vm, struct Screen* screen);

#endif
