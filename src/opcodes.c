#include <stddef.h>
#include <stdint.h>

#include "logging.h"
#include "opcodes.h"
#include "virtual-machine.h"

/**
 * @brief Process the opcode 0.
 *
 * @param opcode The data of the opcode.
 * @param vm The virtual machine to be the changes applied.
 * @param screen The screen to may be cleaned.
 * @return
 */
uint8_t opcode_0(struct Opcode opcode, struct VirtualMachine* vm, struct Screen* screen)
{
    switch (opcode.nibbles_2_3_4) {
    case 0x0E0:
        debug("Cleaning the screen");

        for (size_t y = 0; y < screen->height; y++) {
            for (size_t x = 0; x < screen->width; x++) {
                screen->buffer[y][x] = false;
            }
        }

        if (draw_screen(screen) != 0) {
            return 1;
        }

        return 0;
        break;
    case 0x0EE:
        vm->pc = vm->pc_stack[vm->pc_stack_index - 1];
        debug("Jumping back from subroutine to %#05x", vm->pc);

        vm->pc_stack_index--;

        return 0;
        break;

    default:
        info("Execute machine language routine opcode detected, skipping it (This game may not be compatible with the emulator!)");
        break;
    }

    return 0;
}

void opcode_3_4(struct Opcode opcode, struct VirtualMachine* vm, bool should_be_equal)
{
    if ((vm->v_registers[opcode.nibble_2] == opcode.byte_2) == should_be_equal) {
        debug("Skipped");
        vm->pc += 2;
    }
}

void opcode_5_9(struct Opcode opcode, struct VirtualMachine* vm, bool should_be_equal)
{
    uint8_t register_1 = vm->v_registers[opcode.nibble_2];
    uint8_t register_2 = vm->v_registers[opcode.nibble_3];

    if ((register_1 == register_2) == should_be_equal) {
        debug("Skipped");
        vm->pc += 2;
    }
}

void opcode_8(struct Opcode opcode, struct VirtualMachine* vm)
{
    uint8_t register_1_index = opcode.nibble_2;
    uint8_t register_2_index = opcode.nibble_3;

    uint8_t* register_1 = &(vm->v_registers[register_1_index]);
    uint8_t* register_2 = &(vm->v_registers[register_2_index]);

    switch (opcode.nibble_4) {
    case 0:
        debug("Setting v%d equal to v%d", register_1_index, register_2_index);

        *register_1 = *register_2;
        break;
    case 1:
        debug("Setting v%d equal to v%d | v%d", register_1_index, register_1_index, register_2_index);

        *register_1 |= *register_2;
        vm->v_registers[15] = 0;
        break;
    case 2:
        debug("Setting v%d equal to v%d & v%d", register_1_index, register_1_index, register_2_index);

        *register_1 &= *register_2;
        vm->v_registers[15] = 0;
        break;
    case 3:
        debug("Setting v%d equal to v%d ^ v%d", register_1_index, register_1_index, register_2_index);

        *register_1 ^= *register_2;
        vm->v_registers[15] = 0;
        break;
    case 4: {
        debug("Setting v%d equal to v%d + v%d", register_1_index, register_1_index, register_2_index);

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
        debug("Setting v%d equal to v%d - v%d", register_1_index, register_1_index, register_2_index);

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
        debug("Setting v%d equal to v%d - v%d", register_1_index, register_2_index, register_1_index);

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
        debug("Setting v%d equal to v%d and shifting it once to the right", register_1_index, register_2_index);

        *register_1 = *register_2;
        uint8_t unshift_register_1 = *register_1;

        *register_1 >>= 1;

        vm->v_registers[15] = unshift_register_1 & 0x01;
        break;
    }
    case 0x0E: {
        debug("Setting v%d equal to v%d and shifting it once to the left", register_1_index, register_2_index);

        *register_1 = *register_2;
        uint8_t unshift_register_1 = *register_1;

        *register_1 <<= 1;

        vm->v_registers[15] = (unshift_register_1 & 0x80) >> 7;
        break;
    }
    }
}

uint8_t opcode_d(struct Opcode opcode, struct VirtualMachine* vm, struct Screen* screen)
{
    uint8_t x = vm->v_registers[opcode.nibble_2] % screen->width;
    uint8_t y = vm->v_registers[opcode.nibble_3] % screen->height;

    vm->v_registers[15] = 0;

    for (size_t height = 0; height < opcode.nibble_4 && y + height < screen->height; height++) {
        uint8_t row_data = vm->memory[vm->index_register + height];

        int bit_pos = 0;
        while (bit_pos < 8) {
            if (row_data & 0x80) {
                if ((size_t)(x + bit_pos) >= screen->width) {
                    bit_pos = 8;
                    continue;
                }

                if (screen->buffer[y + height][x + bit_pos]) {
                    vm->v_registers[15] = 1;
                }

                screen->buffer[y + height][x + bit_pos] = !screen->buffer[y + height][x + bit_pos];
            }

            row_data = row_data << 1;
            bit_pos++;
        }
    }

    // printf("Drawing sprite at (%d, %d)\n", x, y);
    if (draw_screen(screen) != 0) {
        return 1;
    }

    return 0;
}
