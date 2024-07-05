#include <SDL2/SDL.h>
#include <stdbool.h>

#include "render.h"

typedef struct Screen Screen;
typedef struct ScreenBuffer ScreenBuffer;

void log_error(char* message)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, message, SDL_GetError());
}

int clear_screen(SDL_Renderer* renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF) != 0) {
        log_error("Couldn't set the draw color: %s");
        return 1;
    }

    SDL_RenderClear(renderer);

    return 0;
}

int draw_pixel(int x, int y, SDL_Renderer* renderer)
{
    SDL_Rect pixel;

    pixel.x = x;
    pixel.y = y;
    pixel.w = 1;
    pixel.h = 1;

    if (SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF) != 0) {
        log_error("Couldn't set the draw color: %s");
        return 1;
    }

    if (SDL_RenderDrawRect(renderer, &pixel) != 0) {
        log_error("Couldn't create pixel: %s");
        return 1;
    }

    return 0;
}

int draw_screen_buffer(ScreenBuffer* screen_buffer, Screen* screen)
{
    clear_screen(screen->renderer);

    for (size_t y = 0; y <= screen_buffer->screen_height; y++) {
        for (size_t x = 0; x <= screen_buffer->screen_width; x++) {
            if (!screen_buffer->buffer[y][x]) {
                continue;
            }

            if (draw_pixel(x, y, screen->renderer) != 0) {
                return 1;
            }
        }
    }

    SDL_RenderPresent(screen->renderer);
    return 0;
}

Screen* create_screen(size_t screen_height, size_t screen_width)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        log_error("error initializing SDL: %s");
        return NULL;
    }

    SDL_Window* window = SDL_CreateWindow("och8S", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, screen_width * 8, screen_height * 8, 0);
    if (window == NULL) {
        log_error("Couldn't create window: %s");
        return NULL;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        log_error("Couldn't create renderer: %s");
        goto renderer_failed;
    }

    if (SDL_RenderSetLogicalSize(renderer, screen_width, screen_height)) {
        log_error("Couldn't set the render logical size");
        goto logical_size_or_screen_failed;
    }

    Screen* screen = malloc(sizeof(Screen));
    if (screen == NULL) {
        puts("Malloc 'screen' failed!");
        goto logical_size_or_screen_failed;
    }

    screen->window = window;
    screen->renderer = renderer;

    return screen;

logical_size_or_screen_failed:
    SDL_DestroyRenderer(renderer);
renderer_failed:
    SDL_DestroyWindow(window);

    return NULL;
}

ScreenBuffer* create_screen_buffer(size_t screen_height, size_t screen_width)
{
    ScreenBuffer* screen_buffer = malloc(sizeof(ScreenBuffer));

    screen_buffer->screen_height = screen_height;
    screen_buffer->screen_width = screen_width;

    screen_buffer->buffer = malloc(sizeof(bool*) * screen_height);
    if (screen_buffer->buffer == NULL) {
        puts("Malloc 'screen_buffer->buffer' failed!");
        goto handle_screen_buffer;
    }

    size_t successful_mallocs_x_arrays = 0;

    for (size_t i = 0; i <= screen_height; i++) {
        screen_buffer->buffer[i] = malloc(sizeof(bool) * screen_width);

        if (screen_buffer->buffer[i] == NULL) {
            puts("Malloc 'screen_buffer->buffer[i]' failed!");

            for (size_t j = 0; j <= successful_mallocs_x_arrays; j++) {
                free(screen_buffer->buffer[i]);
            }

            free(screen_buffer->buffer);

            goto handle_screen_buffer;
        }

        successful_mallocs_x_arrays++;

        for (size_t j = 0; j <= screen_width; j++) {
            screen_buffer->buffer[i][j] = false;
        }
    }

    return screen_buffer;

handle_screen_buffer:
    free(screen_buffer);

    return NULL;
}

void delete_screen_buffer(ScreenBuffer* screen_buffer)
{
    for (size_t y = 0; y <= screen_buffer->screen_height; y++) {
        free(screen_buffer->buffer[y]);
    }

    free(screen_buffer->buffer);
    free(screen_buffer);
}

void delete_screen(Screen* screen)
{
    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);

    free(screen);

    SDL_Quit();
}
