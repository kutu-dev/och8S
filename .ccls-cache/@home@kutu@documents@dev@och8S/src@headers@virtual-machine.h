#ifndef OCH8S_VIRTUAL_MACHINE_H
#define OCH8S_VIRTUAL_MACHINE_H

#include <render.h>
#include <stddef.h>
#include <stdint.h>

typedef struct VirtualMachine {
    uint8_t memory[4098];
    uint16_t index_register;
    uint16_t pc;
    uint8_t v_registers[16];
    uint16_t pc_stack[200];
    size_t pc_stack_index;
    uint8_t delay_timer;
    uint8_t sound_timer;
    int8_t wait_key;
} VirtualMachine;

typedef struct Opcode {
    uint8_t nibble_1;
    uint8_t nibble_2;

    uint8_t byte_2;

    uint8_t nibble_3;
    uint8_t nibble_4;

    uint16_t nibbles_2_3_4;
} Opcode;

Opcode get_opcode(VirtualMachine* virtual_machine);

VirtualMachine* create_virtual_machine(void);

int process_opcode(VirtualMachine* virtual_machine, ScreenBuffer* screen_buffer, Screen* screen);

#endif
