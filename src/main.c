#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "audio.h"
#include "keys.h"
#include "logging.h"
#include "render.h"
#include "save-state.h"
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
        error("Can't get timestamp");
        return 0;
    }

    return timestamp.tv_sec * 100000 + timestamp.tv_nsec / 1000;
}

int main(int argc, char* argv[])
{
    char* rom_path = NULL;
    bool manual_step = false;

    while (optind < argc) {
        int option = getopt(argc, argv, "ds");

        if (option == -1)
        {
          rom_path = argv[optind];
          
          optind++;
          continue;
        }

        switch (option) {
        case 'd':
            debug_enable = true;
            break;
        case 's':
            manual_step = true;
            break;
        default:
            error("Unknown option");

            return 1;
            break;
        }
    }

    if (rom_path == NULL) {
      error("Missing ROM path");
      return 1;
    }

    if (manual_step) {
      warning("Manual step is enabled, press ENTER on the terminal to step once the CPU");
    }

    // TODO Reduce subsystems initialized (remember to also test it on macOS)
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        error("Cound't initialze SDL: %s", SDL_GetError());
        return 1;
    }

    uint32_t audio_sample_counter = 0;
    if (setup_audio(&audio_sample_counter) != 0) {
        goto audio_failed;
    }

    uint16_t opcodes_per_second = 700;

    info("Welcome to och8S emulator!");
    info("CPU Clock: %ldHz", opcodes_per_second);

    srand(time(NULL));

    struct Screen* screen = create_screen(32, 64);
    if (screen == NULL) {
        return 1;
    }

    debug("Screen created");

    draw_screen(screen);

    struct VirtualMachine* vm = create_virtual_machine(rom_path);
    if (vm == NULL) {
        goto virtual_machine_failed;
    }

    debug("Virtual machine created");

    uint64_t cpu_old_time = get_microsecond_timestamp();
    if (cpu_old_time == 0) {
        goto cpu_old_time_failed;
    }

    uint64_t timers_old_time = get_microsecond_timestamp();
    if (timers_old_time == 0) {
        goto timers_old_time_failed;
    }

    debug("Timers set");

    bool quit = false;

    debug("Starting the mainloop");
    while (!quit) {
        uint64_t cpu_new_time = get_microsecond_timestamp();
        if (cpu_new_time == 0) {
            goto cpu_new_time_failed;
        }

        uint64_t timers_new_time = get_microsecond_timestamp();
        if (timers_new_time == 0) {
            goto timers_new_time_failed;
        }

        uint64_t cpu_delta = cpu_new_time - cpu_old_time;
        uint64_t timers_delta = timers_new_time - timers_old_time;

        // The original CHIP-8 spec specify that the sound should start with more that one set in the timer
        if (vm->sound_timer > 1) {
            SDL_PauseAudio(0);
        } else {
            SDL_PauseAudio(1);
            audio_sample_counter = 0;
        }

        if (timers_delta >= 1.0 / 60.0 * 1000000) {
            if (vm->delay_timer > 0) {
                vm->delay_timer--;
            }

            if (vm->sound_timer > 0) {
                vm->sound_timer--;
            }

            timers_old_time = timers_new_time;
        }

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                debug("Quit event detected, closing the emulator");
                quit = true;
            }

            if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    draw_screen(screen);
                }
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.scancode == SDL_SCANCODE_N) {
                    if (save_state(vm, screen) != 0) {
                        return 1;
                    }
                }

                if (event.key.keysym.scancode == SDL_SCANCODE_M) {
                    if (load_state(vm, screen) > 1) {
                        return 1;
                    }
                }
            }

            if (event.type == SDL_KEYUP && vm->wait_key == -1) {
                vm->wait_key = sdl_scancode_to_chip8_key(event.key.keysym.scancode);
            }
        }

        if (manual_step) {
          getchar();
        }
    
        if (cpu_delta >= (1.0 / opcodes_per_second) * 1000000) {
            if (step_cpu(vm, screen) != 0) {
                goto step_cpu_failed;
            }

            cpu_old_time = cpu_new_time;
        }

        SDL_Delay(1);
    }

    delete_screen(screen);
    debug("Deallocated the screen");

    free(vm);
    debug("Deallocated the virtual machine");
    info("Goodbye!");

    SDL_CloseAudio();
    SDL_Quit();

    return 0;

step_cpu_failed:
cpu_new_time_failed:
timers_new_time_failed:
cpu_old_time_failed:
timers_old_time_failed:
    free(vm);
    debug("Deallocated the virtual machine");
virtual_machine_failed:
    delete_screen(screen);
    debug("Deallocated the screen");

    SDL_CloseAudio();
audio_failed:
    SDL_Quit();

    return 1;
}
