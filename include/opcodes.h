#ifndef OCH8S_OPCODES_H
#define OCH8S_OPCODES_H

#include <stdint.h>

#include "render.h"
#include "virtual-machine.h"

uint8_t opcode_0(struct Opcode opcode, struct VirtualMachine* vm, struct Screen* screen);

void opcode_3_4(struct Opcode opcode, struct VirtualMachine* vm, bool should_be_equal);

void opcode_5_9(struct Opcode opcode, struct VirtualMachine* vm, bool should_be_equal);

void opcode_8(struct Opcode opcode, struct VirtualMachine* vm);

uint8_t opcode_d(struct Opcode opcode, struct VirtualMachine* vm, struct Screen* screen);

void opcode_f(struct Opcode opcode, struct VirtualMachine* vm);

#endif
