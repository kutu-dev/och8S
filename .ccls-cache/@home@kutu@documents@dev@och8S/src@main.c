#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "keys.h"
#include "render.h"
#include "virtual-machine.h"

/**
 * @brief Get in microseconds a timestamp of the current UTC time
 *
 * @return The timestamp in microseconds
 */
uint64_t get_microsecond_timestamp()
{
    struct timespec timestamp;

    if (timespec_get(&timestamp, TIME_UTC) == 0) {
        // TODO: LOGGING
        printf("ERROR: Can't get timestamp");
        return 0;
    }

    return timestamp.tv_sec * 100000 + timestamp.tv_nsec / 1000;
}

int8_t main()
{
    srand(time(NULL));

    Screen* screen = create_screen(32, 64);
    if (screen == NULL) {
        return 1;
    }

    VirtualMachine* virtual_machine = create_virtual_machine();
    if (virtual_machine == NULL) {
        return 1;
    }

    draw_screen_buffer(screen);

    uint64_t cpu_old_time = get_microsecond_timestamp();
    if (cpu_old_time == 0) {
        return 1;
    }

    uint64_t timers_old_time = get_microsecond_timestamp();
    if (timers_old_time == 0) {
        return 1;
    }

    bool quit = false;

    while (!quit) {
        uint64_t cpu_new_time = get_microsecond_timestamp();
        if (cpu_new_time == 0) {
            return 1;
        }

        uint64_t timers_new_time = get_microsecond_timestamp();
        if (timers_new_time == 0) {
            return 1;
        }

        uint64_t cpu_delta = cpu_new_time - cpu_old_time;
        uint64_t timers_delta = timers_new_time - timers_old_time;

        if (timers_delta >= 17000) {
            if (virtual_machine->delay_timer > 0) {
                virtual_machine->delay_timer--;
            }

            if (virtual_machine->sound_timer > 0) {
                virtual_machine->sound_timer--;
            }

            timers_old_time = timers_new_time;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    draw_screen_buffer(screen_buffer, screen);
                }
            }

            if (event.type == SDL_KEYUP && virtual_machine->wait_key == -1) {
                virtual_machine->wait_key = sdl_scancode_to_chip8_key(event.key.keysym.scancode);
            }
        }

        if (cpu_delta >= 1400) {
            if (process_opcode(virtual_machine, screen_buffer, screen) != 0) {
                return 1;
            }

            cpu_old_time = cpu_new_time;
        }

        SDL_Delay(1);
    }

    delete_screen_buffer(screen_buffer);
    delete_screen(screen);
}
