#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "keys.h"
#include "logging.h"
#include "opcodes.h"
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
 * @brief Create a new virtual machine.
 *
 * @return The created virtual machine. It can (and MUST) be deallocated after its use with `free()`.
 */
struct VirtualMachine* create_virtual_machine()
{
    struct VirtualMachine* vm = malloc(sizeof(struct VirtualMachine));

    if (vm == NULL) {
        error("Malloc 'vm' failed");
        return NULL;
    }

    // Clear the memory region of the virtual machine
    // Caution!: This will only work if the struct if made only of integer types
    memset(vm, 0, sizeof(struct VirtualMachine));

    vm->pc = 0x200;

    memcpy(vm->memory + 0x050, font_data, sizeof(font_data));

    FILE* rom = fopen("rom.ch8", "rb");

    // Get the size of the file in bytes
    fseek(rom, 0, SEEK_END);
    long size = ftell(rom);
    rewind(rom);

    vm->wait_key = -2;

    size_t count = fread(vm->memory + 0x200, sizeof(vm->memory[0]), size, rom);

    if (count != (size_t)size && ferror(rom) != 0) {
        error("Reading rom failed!");

        free(vm);
        vm = NULL;
    }

    fclose(rom);

    return vm;
}

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

    // debug("Opcode: %x, X: %x, Y: %x, N: %x, NN: %02x, NNN: %03x", opcode.nibble_1, opcode.nibble_2, opcode.nibble_3, opcode.nibble_4, opcode.byte_2, opcode.nibbles_2_3_4);

    switch (opcode.nibble_1) {
    case 0x0:
        if (opcode_0(opcode, vm, screen) != 0) {
            return 1;
        }

        break;

    case 0x1:
        vm->pc = opcode.nibbles_2_3_4;
        debug("Jumping to %#05x", vm->pc);

        break;

    case 0x2:
        vm->pc_stack[vm->pc_stack_index] = vm->pc;
        vm->pc_stack_index++;

        vm->pc = opcode.nibbles_2_3_4 / sizeof(vm->memory[0]);
        debug("Jumping to subroutine at %#05x", vm->pc);

        break;

    case 0x3:
        debug("Skipping if vX equals NN");
        opcode_3_4(opcode, vm, true);
        break;

    case 0x4:
        debug("Skipping if vX not equals NN");
        opcode_3_4(opcode, vm, false);
        break;

    case 0x5: {
        debug("Skipping if vX equals vY");
        opcode_5_9(opcode, vm, true);
        break;
    }

    case 0x6:
        debug("Setting register v%x to value %#04x", opcode.nibble_2, opcode.byte_2);
        vm->v_registers[opcode.nibble_2] = opcode.byte_2;

        break;

    case 0x7:
        debug("Adding %d to register v%x", opcode.byte_2, opcode.nibble_2);
        vm->v_registers[opcode.nibble_2] += opcode.byte_2;

        break;

    case 0x8:
        opcode_8(opcode, vm);
        break;

    case 0x9:
        debug("Skipping if vX not equals vY");
        opcode_5_9(opcode, vm, false);
        break;

    case 0xA:
        debug("Setting register i to value %#05x", opcode.nibbles_2_3_4);
        vm->index_register = opcode.nibbles_2_3_4;

        break;

    case 0xB:
        vm->pc = opcode.nibbles_2_3_4 + vm->v_registers[0];
        debug("Jumping to %#05x (%#05x + %#05x)", vm->pc, opcode.nibbles_2_3_4, vm->v_registers[0]);

        break;

    case 0xC:
        debug("Generating a random number an setting it to v%d", opcode.nibble_2);
        vm->v_registers[opcode.nibble_2] = rand() & opcode.byte_2;

        break;

    case 0x0D:
        if (opcode_d(opcode, vm, screen) != 0) {
            return 1;
        }

        break;

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
            // puts("Saving all V registers to memory");
            for (size_t i = 0; i <= opcode.nibble_2; i++) {
                vm->memory[vm->index_register + i] = vm->v_registers[i];
            }

            vm->index_register += opcode.nibble_2 + 1;
            break;

        case 0x65:
            // puts("Dumping all V registers from memory");
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
