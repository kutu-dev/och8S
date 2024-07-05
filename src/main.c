#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "render.h"
#include "virtual-machine.h"

int main()
{
    srand(time(NULL));

    Screen* screen = create_screen(32, 64);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    SDL_SetHint(SDL_HINT_RENDER_LOGICAL_SIZE_MODE, "letterbox");
    if (screen == NULL) {
        return 1;
    }

    ScreenBuffer* screen_buffer = create_screen_buffer(32, 64);

    if (screen_buffer == NULL) {
        return 1;
    }

    VirtualMachine* virtual_machine = create_virtual_machine();

    if (virtual_machine == NULL) {
        return 1;
    }

    draw_screen_buffer(screen_buffer, screen);

    uint8_t key_pressed = 100;

    bool quit = false;

    while (!quit) {
        SDL_Delay(1);

        if (virtual_machine -> delay_timer > 0) {
          virtual_machine -> delay_timer--;
        }

        if (virtual_machine -> sound_timer > 0) {
          virtual_machine -> sound_timer--;
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

            if (event.type == SDL_KEYUP) {
              key_pressed = 100;
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.repeat != 0) {
                  continue;
                }

                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_1:
                    key_pressed = 1;
                    break;
                case SDL_SCANCODE_2:
                    key_pressed = 2;
                    break;
                case SDL_SCANCODE_3:
                    key_pressed = 3;
                    break;

                case SDL_SCANCODE_4:
                    key_pressed = 12;
                    break;

                default:
                    key_pressed = 100;
                    break;
                }
            }
        }

        // printf("%d\n", key_pressed);

        if (process_opcode(&key_pressed, virtual_machine, screen_buffer, screen) != 0) {
            return 1;
        }
    }

    delete_screen_buffer(screen_buffer);
    delete_screen(screen);
}
