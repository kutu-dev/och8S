#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "render.h"
#include "virtual-machine.h"

int main()
{
    Screen* screen = create_screen(32, 64);

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
    while (1) {
        SDL_Event event;
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        if(process_opcode(virtual_machine, screen_buffer, screen) != 0) {
            return 1;
        }
    }

    delete_screen_buffer(screen_buffer);
    delete_screen(screen);
}
