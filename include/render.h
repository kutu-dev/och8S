#ifndef OCH8S_RENDER_H
#define OCH8S_RENDER_H

#include <SDL2/SDL.h>
#include <stdbool.h>

struct Screen {
    SDL_Window* window;
    SDL_Renderer* renderer;

    size_t height;
    size_t width;

    bool** buffer;
};

uint8_t draw_screen(struct Screen* screen);

struct Screen* create_screen(size_t screen_height, size_t screen_width);

void delete_screen(struct Screen* screen);

#endif
