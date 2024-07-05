#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "virtual-machine.h"

typedef struct VirtualMachine VirtualMachine;
typedef struct Opcode Opcode;

static uint8_t font[16 * 5] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

Opcode get_opcode(VirtualMachine* virtual_machine)
{
    Opcode opcode;

    opcode.nibble_1 = virtual_machine->memory[virtual_machine->pc] >> 4;
    opcode.nibble_2 = virtual_machine->memory[virtual_machine->pc] & 0x0F;

    opcode.byte_2 = virtual_machine->memory[virtual_machine->pc + 1];

    opcode.nibble_3 = virtual_machine->memory[virtual_machine->pc + 1] >> 4;
    opcode.nibble_4 = virtual_machine->memory[virtual_machine->pc + 1] & 0x0F;

    opcode.nibbles_2_3_4 = opcode.nibble_2 << 8 | opcode.byte_2;

    return opcode;
}

VirtualMachine* create_virtual_machine()
{
    VirtualMachine* virtual_machine = malloc(sizeof(VirtualMachine));

    if (virtual_machine == NULL) {
        puts("Malloc 'virtual_machine' failed!");
        return NULL;
    }

    // Clear the memory region of the virtual machine
    memset(virtual_machine, 0, sizeof(VirtualMachine));

    // Offset the PC the reserved memory region for VM data
    virtual_machine->pc = 512 / sizeof(virtual_machine->memory[0]);

    // Copy the font to the memory at `050` (80 bits)
    memcpy(virtual_machine->memory + 80 / sizeof(virtual_machine->memory[0]), font, sizeof(font));

    FILE* rom = fopen("rom.ch8", "rb");

    // Get the size of the file in bytes
    fseek(rom, 0, SEEK_END);
    long size = ftell(rom);
    rewind(rom);

    // Copy the ROM offset to the address 200
    size_t count = fread(virtual_machine->memory + 512 / sizeof(virtual_machine->memory[0]), sizeof(virtual_machine->memory[0]), size, rom);

    if (count != (size_t)size && ferror(rom) != 0) {
        puts("Reading rom failed!");

        free(virtual_machine);
        virtual_machine = NULL;
    }

    fclose(rom);

    return virtual_machine;
}

int process_opcode(VirtualMachine* virtual_machine, ScreenBuffer* screen_buffer, Screen* screen)
{
    Opcode opcode = get_opcode(virtual_machine);

    printf("%x %x %x %x -- %x at %x\n", opcode.nibble_1, opcode.nibble_2, opcode.nibble_3, opcode.nibble_4, opcode.nibbles_2_3_4, virtual_machine->pc);

    virtual_machine->pc += 2;

    switch (opcode.nibble_1) {
    case 0x00:
        if (opcode.nibbles_2_3_4 == 0x0E0) {
            puts("Clearing the screen");

            for (size_t y = 0; y <= screen_buffer->screen_height; y++) {
                for (size_t x = 0; x <= screen_buffer->screen_width; x++) {
                    screen_buffer->buffer[y][x] = false;
                }
            }

            if (draw_screen_buffer(screen_buffer, screen) != 0) {
                return 1;
            }
        }

        if (opcode.nibbles_2_3_4 == 0x0EE) {
            virtual_machine->pc = virtual_machine->pc_stack[virtual_machine->pc_stack_index];
            virtual_machine->pc_stack_index--;
        }

        puts("Execute machine language routine detected, skipping it");
        break;

    case 0x01:
        printf("Jumping to address %x\n", opcode.nibbles_2_3_4);
        virtual_machine->pc = opcode.nibbles_2_3_4 / sizeof(virtual_machine->memory[0]);

        break;

    case 0x02:
        printf("Jumping to subroutine at %x\n", opcode.nibbles_2_3_4);

        virtual_machine->pc_stack[virtual_machine->pc_stack_index] = virtual_machine->pc;

        virtual_machine->pc = opcode.nibbles_2_3_4 / sizeof(virtual_machine->memory[0]);

        break;

    case 0x03:
        if (virtual_machine->v_registers[opcode.nibble_2] == opcode.byte_2) {
            virtual_machine->pc += 2;
        }
        break;

    case 0x04:
        if (virtual_machine->v_registers[opcode.nibble_2] != opcode.byte_2) {
            virtual_machine->pc += 2;
        }
        break;

    case 0x05: {
        uint8_t register_1 = virtual_machine->v_registers[opcode.nibble_2];
        uint8_t register_2 = virtual_machine->v_registers[opcode.nibble_3];

        if (register_1 == register_2) {
            virtual_machine->pc += 2;
        }

        break;
    }

    case 0x08:
        switch (opcode.nibble_4) {
        case 0:
            virtual_machine->v_registers[opcode.nibble_2] = virtual_machine->v_registers[opcode.nibble_3];
            break;
        }
        break;

    case 0x09: {
        uint8_t register_1 = virtual_machine->v_registers[opcode.nibble_2];
        uint8_t register_2 = virtual_machine->v_registers[opcode.nibble_3];

        if (register_1 != register_2) {
            virtual_machine->pc += 2;
        }

        break;
    }

    case 0x06:
        printf("Setting register %x to value %x\n", opcode.nibble_2, opcode.byte_2);

        virtual_machine->v_registers[opcode.nibble_2] = opcode.byte_2;

        break;

    case 0x07:
        printf("Adding %x to register %x\n", opcode.byte_2, opcode.nibble_2);

        virtual_machine->v_registers[opcode.nibble_2] += opcode.byte_2;

        break;

    case 0x0A:
        printf("Setting I register to value %x\n", opcode.nibbles_2_3_4);
        virtual_machine->index_register = opcode.nibbles_2_3_4;

        break;

    case 0x0D: {
        uint8_t x = virtual_machine->v_registers[opcode.nibble_2] % screen_buffer->screen_width;
        uint8_t y = virtual_machine->v_registers[opcode.nibble_3] % screen_buffer->screen_height;

        virtual_machine->v_registers[15] = 0;

        for (size_t height = 0; height < opcode.nibble_4; height++) {
            uint8_t row_data = virtual_machine->memory[virtual_machine->index_register + height];

            int bit_pos = 0;
            while (bit_pos < 8) {
                if (row_data & 0x80) {
                    if (screen_buffer->buffer[y + height][x + bit_pos]) {
                        virtual_machine->v_registers[15] = 1;
                    }

                    screen_buffer->buffer[y + height][x + bit_pos] = !screen_buffer->buffer[y + height][x + bit_pos];
                }

                row_data = row_data << 1;
                bit_pos++;
            }
        }

        printf("Drawing sprite at (%d, %d)\n", x, y);
        if (draw_screen_buffer(screen_buffer, screen) != 0) {
            return 1;
        }

        break;
    }
    }

    return 0;
}
