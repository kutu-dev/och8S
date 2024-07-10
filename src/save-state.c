#include <SDL2/SDL.h>

#include "logging.h"
#include "render.h"
#include "save-state.h"
#include "virtual-machine.h"

/**
 * @brief Get the path where the savestate should be saved and loaded from.
 *
 * @return A pointer to the path where the savestate is located. It should be `freed` after its use.
 */
char* get_savestate_path()
{
    char* pref_path = SDL_GetPrefPath("kutu-dev", "och8S");

    const char* savestate_filename = "savestate.dat";

    char* savestate_path = malloc(strlen(pref_path) + strlen(savestate_filename) + 1);

    strcpy(savestate_path, pref_path);
    strcat(savestate_path, savestate_filename);

    return savestate_path;
}

/**
 * @brief Save the virtual machine and screen state to a savestate file.
 *
 * @param vm The virtual machine to get its data from.
 * @param screen The screen to get its pixel data from.
 * @return Return 0 on success or another number on failure.
 */
uint8_t save_state(struct VirtualMachine* vm, struct Screen* screen)
{
    char* savestate_path = get_savestate_path();
    FILE* f = fopen(savestate_path, "wb");

    if (fwrite(vm->memory, sizeof(vm->memory[0]), sizeof(vm->memory), f) < sizeof(vm->memory)) {
        error("The memory wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->pc, sizeof(vm->pc), 1, f) < 1) {
        error("The PC wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(vm->pc_stack, sizeof(vm->pc_stack[0]), sizeof(vm->pc_stack), f) < sizeof(vm->pc_stack)) {
        error("The PC stack wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->pc_stack_index, sizeof(vm->pc_stack_index), 1, f) < 1) {
        error("The PC stack index wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->index_register, sizeof(vm->index_register), 1, f) < 1) {
        error("The register i wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(vm->v_registers, sizeof(vm->v_registers[0]), sizeof(vm->v_registers), f) < sizeof(vm->v_registers)) {
        error("The registers v stack weren't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->delay_timer, sizeof(vm->delay_timer), 1, f) < 1) {
        error("The delay timer wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->sound_timer, sizeof(vm->sound_timer), 1, f) < 1) {
        error("The sound timer wasn't able to be fully written into the save state");
        goto write_failed;
    }

    if (fwrite(&vm->wait_key, sizeof(vm->wait_key), 1, f) < 1) {
        error("The wait key wasn't able to be fully written into the save state");
        goto write_failed;
    }

    for (size_t i = 0; i < screen->height; i++) {
        if (fwrite(screen->buffer[i], sizeof(screen->buffer[i][0]), screen->width, f) < screen->width) {
            error("The screen wasn't able to be fully written into the save state");
            goto write_failed;
        }
    }

    fclose(f);
    free(savestate_path);
    return 0;

write_failed:
    fclose(f);
    free(savestate_path);
    return 1;
}

/**
 * @brief Apply the data from a savestate file to the virtual machine and screen.
 *
 * @param vm The virtual machine where the data should be written to.
 * @param screen The screen where the pixel data should be written to.
 * @return Return 0 on success, 1 if opening the savestate file fails or another number on failure.
 */
uint8_t load_state(struct VirtualMachine* vm, struct Screen* screen)
{
    char* savestate_path = get_savestate_path();
    FILE* f = fopen(savestate_path, "rb");

    if (f == NULL) {
        error("Savestate file is missing or access has been refused by permission configurations");
        return 1;
    }

    if (fread(vm->memory, sizeof(vm->memory[0]), sizeof(vm->memory), f) < sizeof(vm->memory)) {
        error("The memory wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->pc, sizeof(vm->pc), 1, f) < 1) {
        error("The PC wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(vm->pc_stack, sizeof(vm->pc_stack[0]), sizeof(vm->pc_stack), f) < sizeof(vm->pc_stack)) {
        error("The PC stack wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->pc_stack_index, sizeof(vm->pc_stack_index), 1, f) < 1) {
        error("The PC stack index wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->index_register, sizeof(vm->index_register), 1, f) < 1) {
        error("The register i wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(vm->v_registers, sizeof(vm->v_registers[0]), sizeof(vm->v_registers), f) < sizeof(vm->v_registers)) {
        error("The registers v stack weren't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->delay_timer, sizeof(vm->delay_timer), 1, f) < 1) {
        error("The delay timer wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->sound_timer, sizeof(vm->sound_timer), 1, f) < 1) {
        error("The sound timer wasn't able to be fully read from the save state");
        goto read_failed;
    }

    if (fread(&vm->wait_key, sizeof(vm->wait_key), 1, f) < 1) {
        error("The wait key wasn't able to be fully read from the save state");
        goto read_failed;
    }

    for (size_t i = 0; i < screen->height; i++) {
        if (fread(screen->buffer[i], sizeof(screen->buffer[i][0]), screen->width, f) < screen->width) {
            error("The screen wasn't able to be fully read from the save state");
            goto read_failed;
        }
    }

    draw_screen(screen);

    fclose(f);
    free(savestate_path);
    return 0;

read_failed:
    fclose(f);
    free(savestate_path);
    return 2;
}
