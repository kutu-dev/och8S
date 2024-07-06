#ifndef OCH8S_VIRTUAL_MACHINE_H
#define OCH8S_VIRTUAL_MACHINE_H

#include <render.h>
#include <stddef.h>
#include <stdint.h>

struct VirtualMachine {
    uint8_t memory[4098];

    uint16_t pc;
    uint16_t pc_stack[200];
    size_t pc_stack_index;

    uint16_t index_register;
    uint8_t v_registers[16];

    uint8_t delay_timer;
    uint8_t sound_timer;

    int8_t wait_key;
};

struct Opcode {
    uint8_t nibble_1;
    uint8_t nibble_2;

    uint8_t byte_2;

    uint8_t nibble_3;
    uint8_t nibble_4;

    uint16_t nibbles_2_3_4;
};

struct Opcode get_opcode(struct VirtualMachine* vm);

struct VirtualMachine* create_virtual_machine(void);

uint8_t step_cpu(struct VirtualMachine* vm, struct Screen* screen);

#endif
