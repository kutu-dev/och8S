#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys.h"
#include "render.h"
#include "virtual-machine.h"

struct VirtualMachine;
struct Opcode;

static constexpr uint8_t font_data[] = {
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

/**
 * @brief Parse the opcode located at the PC of the Virtual Machine.
 *
 * @param vm The virtual machine to get from it the memory and PC.
 * @return The parsed opcode info.
 */
struct Opcode get_opcode(struct VirtualMachine* vm)
{
    struct Opcode opcode;

    opcode.nibble_1 = vm->memory[vm->pc] >> 4;
    opcode.nibble_2 = vm->memory[vm->pc] & 0x0F;

    opcode.byte_2 = vm->memory[vm->pc + 1];

    opcode.nibble_3 = vm->memory[vm->pc + 1] >> 4;
    opcode.nibble_4 = vm->memory[vm->pc + 1] & 0x0F;

    opcode.nibbles_2_3_4 = opcode.nibble_2 << 8 | opcode.byte_2;

    return opcode;
}

/**
 * @brief Create a new virtual machine.
 *
 * @return The created virtual machine. It can (and MUST) be deallocated after its use with `free()`.
 */
struct VirtualMachine* create_virtual_machine()
{
    struct VirtualMachine* vm = malloc(sizeof(struct VirtualMachine));

    if (vm == NULL) {
        // TODO CHECK LOGGING
        puts("Malloc 'virtual_machine' failed!");
        return NULL;
    }

    // Clear the memory region of the virtual machine
    memset(vm, 0, sizeof(struct VirtualMachine));

    // Offset the PC the reserved memory region for VM data
    // TODO CHECK WITH HEX
    vm->pc = 512 / sizeof(vm->memory[0]);

    // Copy the font to the memory at `050` (80 bits)
    // TODO CHECK WITH HEX
    memcpy(vm->memory + 80 / sizeof(vm->memory[0]), font_data, sizeof(font_data));

    FILE* rom = fopen("rom.ch8", "rb");

    // Get the size of the file in bytes
    fseek(rom, 0, SEEK_END);
    // TODO CHECK AND FIX TYPE (AVOID CAST BELOW)
    long size = ftell(rom);
    rewind(rom);

    vm->wait_key = -2;

    // TODO CHECK WITH HEX
    // Copy the ROM offset to the address 200
    size_t count = fread(vm->memory + 512 / sizeof(vm->memory[0]), sizeof(vm->memory[0]), size, rom);

    if (count != (size_t)size && ferror(rom) != 0) {
        puts("Reading rom failed!");

        free(vm);
        vm = NULL;
    }

    fclose(rom);

    return vm;
}

/**
 * @brief Step the cpu of the virtual machine one time.
 *
 * @param vm The virtual machine of whose cpu should be step.
 * @param screen The screen where virtual machine state changes may be reflected.
 * @return 
 */
uint8_t step_cpu(struct VirtualMachine* vm, struct Screen* screen)
{
    struct Opcode opcode = get_opcode(vm);

    vm->pc += 2;

    switch (opcode.nibble_1) {
    case 0x00:
        if (opcode.nibbles_2_3_4 == 0x0E0) {
            puts("Clearing the screen");

            for (size_t y = 0; y <= screen->height; y++) {
                for (size_t x = 0; x <= screen->width; x++) {
                    screen->buffer[y][x] = false;
                }
            }

            if (draw_screen(screen) != 0) {
                return 1;
            }

            return 0;
        }

        if (opcode.nibbles_2_3_4 == 0x0EE) {
            vm->pc = vm->pc_stack[vm->pc_stack_index - 1];
            printf("Jumping back to execution at %x, index: %ld\n", vm->pc, vm->pc_stack_index);
            vm->pc_stack_index--;

            return 0;
        }

        puts("Execute machine language routine detected, skipping it");
        break;

    case 0x01:
        // printf("Jumping to address %x\n", opcode.nibbles_2_3_4);
        vm->pc = opcode.nibbles_2_3_4 / sizeof(vm->memory[0]);

        break;

    case 0x02:
        // printf("Jumping to subroutine at %x\n", opcode.nibbles_2_3_4);
        // printf("Sending to stack index: %ld with value %x\n", vm->pc_stack_index, vm->pc);

        vm->pc_stack[vm->pc_stack_index] = vm->pc;
        vm->pc_stack_index++;

        vm->pc = opcode.nibbles_2_3_4 / sizeof(vm->memory[0]);

        break;

    case 0x03:
        if (vm->v_registers[opcode.nibble_2] == opcode.byte_2) {
            vm->pc += 2;
        }
        break;

    case 0x04:
        if (vm->v_registers[opcode.nibble_2] != opcode.byte_2) {
            vm->pc += 2;
        }
        break;

    case 0x05: {
        uint8_t register_1 = vm->v_registers[opcode.nibble_2];
        uint8_t register_2 = vm->v_registers[opcode.nibble_3];

        if (register_1 == register_2) {
            vm->pc += 2;
        }

        break;
    }

    case 0x06:
        // printf("Setting register %x to value %x\n", opcode.nibble_2, opcode.byte_2);

        vm->v_registers[opcode.nibble_2] = opcode.byte_2;

        break;

    case 0x07:
        // printf("Adding %x to register %x\n", opcode.byte_2, opcode.nibble_2);

        vm->v_registers[opcode.nibble_2] += opcode.byte_2;

        break;

    case 0x08: {
        uint8_t* register_1 = &(vm->v_registers[opcode.nibble_2]);
        uint8_t* register_2 = &(vm->v_registers[opcode.nibble_3]);

        switch (opcode.nibble_4) {
        case 0:
            *register_1 = *register_2;
            break;
        case 1:
            *register_1 |= *register_2;
            vm->v_registers[15] = 0;
            break;
        case 2:
            *register_1 &= *register_2;
            vm->v_registers[15] = 0;
            break;
        case 3:
            *register_1 ^= *register_2;
            vm->v_registers[15] = 0;
            break;
        case 4: {
            uint8_t old_register_1 = *register_1;

            *register_1 += *register_2;

            // Detect overflow
            if (*register_1 < old_register_1) {
                vm->v_registers[15] = 1;
            } else {
                vm->v_registers[15] = 0;
            }

            break;
        }
        case 5: {
            bool underflow = *register_1 >= *register_2;

            *register_1 -= *register_2;

            if (underflow) {
                vm->v_registers[15] = 1;
            } else {
                vm->v_registers[15] = 0;
            }
            break;
        }
        case 7: {
            bool underflow = *register_2 >= *register_1;

            *register_1 = *register_2 - *register_1;

            if (underflow) {
                vm->v_registers[15] = 1;
            } else {
                vm->v_registers[15] = 0;
            }
            break;
        }
        case 6: {
            *register_1 = *register_2;
            uint8_t unshift_register_1 = *register_1;

            *register_1 >>= 1;

            vm->v_registers[15] = unshift_register_1 & 0x01;
            break;
        }
        case 0x0E: {
            *register_1 = *register_2;
            uint8_t unshift_register_1 = *register_1;

            *register_1 <<= 1;

            vm->v_registers[15] = (unshift_register_1 & 0x80) >> 7;
            break;
        }
        }
        break;
    }

    case 0x09: {
        uint8_t register_1 = vm->v_registers[opcode.nibble_2];
        uint8_t register_2 = vm->v_registers[opcode.nibble_3];

        if (register_1 != register_2) {
            vm->pc += 2;
        }

        break;
    }

    case 0x0A:
        // printf("Setting I register to value %x\n", opcode.nibbles_2_3_4);
        vm->index_register = opcode.nibbles_2_3_4;

        break;

    case 0xB:
        vm->pc = opcode.nibbles_2_3_4 + vm->v_registers[0];
        break;

    case 0xC: {
        vm->v_registers[opcode.nibble_2] = rand() & opcode.byte_2;
        break;
    }

    case 0x0D: {
        uint8_t x = vm->v_registers[opcode.nibble_2] % screen->width;
        uint8_t y = vm->v_registers[opcode.nibble_3] % screen->height;

        vm->v_registers[15] = 0;

        for (size_t height = 0; height < opcode.nibble_4 && y + height < screen->height; height++) {
            uint8_t row_data = vm->memory[vm->index_register + height];

            int bit_pos = 0;
            while (bit_pos < 8) {
                if (row_data & 0x80) {
                    if (screen->buffer[y + height][x + bit_pos]) {
                        vm->v_registers[15] = 1;
                    }

                    screen->buffer[y + height][x + bit_pos] = !screen->buffer[y + height][x + bit_pos];
                }

                row_data = row_data << 1;
                bit_pos++;
            }
        }

        printf("Drawing sprite at (%d, %d)\n", x, y);
        if (draw_screen(screen) != 0) {
            return 1;
        }

        break;
    }

    case 0x0E: {
        uint8_t requested_key = vm->v_registers[opcode.nibble_2];
        const uint8_t* pressed_keys = SDL_GetKeyboardState(NULL);

        if (opcode.byte_2 == 0x9E) {
            if (pressed_keys[chip8_key_to_sdl_scancode[requested_key]]) {
                vm->pc += 2;
            }
        }

        if (opcode.byte_2 == 0xA1) {

            if (!pressed_keys[chip8_key_to_sdl_scancode[requested_key]]) {
                vm->pc += 2;
            }
        }
        break;
    }

    case 0x0F:
        switch (opcode.byte_2) {
        case 0x07:
            vm->v_registers[opcode.nibble_2] = vm->delay_timer;
            break;

        case 0x15:
            vm->delay_timer = vm->v_registers[opcode.nibble_2];
            break;

        case 0x18:
            vm->sound_timer = vm->v_registers[opcode.nibble_2];
            break;

        case 0x1E:
            vm->index_register += vm->v_registers[opcode.nibble_2];
            break;

        case 0x0A: {
            if (vm->wait_key == -2) {
                vm->wait_key = -1;
                vm->pc -= 2;
                break;
            }

            if (vm->wait_key == -1) {
                vm->pc -= 2;
                break;
            }

            vm->v_registers[opcode.nibble_2] = vm->wait_key;
            vm->wait_key = -2;

            break;
        }

        case 0x29: {
            uint8_t key = vm->v_registers[opcode.nibble_2];

            // Multiply the key by the number of bytes each char takes in memory
            vm->index_register = 80 / sizeof(vm->memory[0]) + key * 5;

            break;
        }

        case 0x33: {
            uint8_t register_1 = vm->v_registers[opcode.nibble_2];

            vm->memory[vm->index_register] = register_1 / 100;
            vm->memory[vm->index_register + 1] = (register_1 / 10) % 10;
            vm->memory[vm->index_register + 2] = register_1 % 10;
            break;
        }

        case 0x55:
            puts("Saving all V registers to memory");
            for (size_t i = 0; i <= opcode.nibble_2; i++) {
                vm->memory[vm->index_register + i] = vm->v_registers[i];
            }

            vm->index_register += opcode.nibble_2 + 1;
            break;

        case 0x65:
            puts("Dumping all V registers from memory");
            for (size_t i = 0; i <= opcode.nibble_2; i++) {
                vm->v_registers[i] = vm->memory[vm->index_register + i];
            }

            vm->index_register += opcode.nibble_2 + 1;
            break;
        }
        break;
    }

    return 0;
}
