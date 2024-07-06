#include <SDL2/SDL.h>
#include <stdbool.h>

#include "render.h"

struct Screen;

void log_error(char* message)
{
    // TODO: LOGGING
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, message, SDL_GetError());
}

/**
 * @brief Clear the renderer.
 *
 * @param renderer The renderer to be cleared.
 * @return Return 0 on success or another number on failure.
 */
uint8_t clear_renderer(SDL_Renderer* renderer)
{
    if (SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF) != 0) {
        // TODO: LOGGING
        log_error("Couldn't set the draw color: %s");
        return 1;
    }

    if (SDL_RenderClear(renderer) != 0) {
      // TODO: LOGGING
      log_error("Couldn't clear the render");
      return 2;
    }

    return 0;
}

/**
 * @brief Draw a "virtual pixeL" (a 1:1 aspect ratio rectangle) to the render.
 *
 * @param x The x position of the pixel.
 * @param y The y position of the pixel.
 * @param renderer The renderer where the pixel should be draw.
 * @return Return 0 on success or another number on failure.
 */
uint8_t draw_pixel(uint8_t x, uint8_t y, SDL_Renderer* renderer)
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
        return 2;
    }

    return 0;
}

/**
 * @brief Draw to the screen the pixel defined in the buffer.
 *
 * @param screen The screen to be the pixels draw.
 * @return Return 0 on success or another number on failure.
 */
uint8_t draw_screen(struct Screen* screen)
{
    clear_renderer(screen->renderer);

    for (size_t y = 0; y <= screen->height; y++) {
        for (size_t x = 0; x <= screen->width; x++) {
            if (!screen->buffer[y][x]) {
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

/**
 * @brief Create a new screen.
 *
 * @param screen_height The height of the screen to create.
 * @param screen_width The width of the screen to create.
 * @return The pointer to the screen or a NULL pointer if an error occurs.
 *  The screen should be freed using the function `delete_screen()`.
 */
struct Screen* create_screen(size_t screen_height, size_t screen_width)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        //TODO LOGGING
        log_error("error initializing SDL: %s");
        return NULL;
    }

    SDL_Window* window = SDL_CreateWindow("och8S", SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, 0);
    if (window == NULL) {
        //TODO LOGGING
        log_error("Couldn't create window: %s");
        return NULL;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        //TODO LOGGING
        log_error("Couldn't create renderer: %s");
        goto renderer_failed;
    }

    if (SDL_RenderSetLogicalSize(renderer, screen_width, screen_height)) {
        //TODO LOGGING
        log_error("Couldn't set the render logical size");
        goto set_logical_size_failed;
    }

    struct Screen* screen = malloc(sizeof(struct Screen));
    if (screen == NULL) {
      // TODO LOGGING
      puts("SCREEN MALLOC FAILED");
      goto screen_failed;
    }

    screen->window = window;
    screen->renderer = renderer;

    screen->height = screen_height;
    screen->width = screen_width;

    screen->buffer = malloc(sizeof(bool*) * screen_height);
    if (screen->buffer == NULL) {
        // TODO LOGGING
        puts("Malloc 'screen_buffer->buffer' failed!");
        goto buffer_failed;
    }

    size_t successfully_allocated_buffer_subarrays = 0;

    for (size_t i = 0; i <= screen_height; i++) {
        screen->buffer[i] = malloc(sizeof(bool) * screen_width);
        if (screen->buffer[i] == NULL) {
            // TODO LOGGING
            puts("Malloc 'screen_buffer->buffer[i]' failed!");
            goto buffer_subarray_failed;
        }

        successfully_allocated_buffer_subarrays++;

        for (size_t j = 0; j <= screen_width; j++) {
            screen->buffer[i][j] = false;
        }
    }

    return screen;

buffer_subarray_failed:
  for (size_t i = 0; i < successfully_allocated_buffer_subarrays; i++) {
      free(screen->buffer[i]);
  }

  free(screen->buffer);
buffer_failed:
  free(screen);
screen_failed:
set_logical_size_failed:
    SDL_DestroyRenderer(renderer);
renderer_failed:
    SDL_DestroyWindow(window);

    return NULL;
}

/**
 * @brief Safely deallocated a screen.
 *
 * @param screen The screen to be deallocated.
 */
void delete_screen(struct Screen* screen)
{
    for (size_t y = 0; y <= screen->height; y++) {
        free(screen->buffer[y]);
    }

    free(screen->buffer);

    SDL_DestroyRenderer(screen->renderer);
    SDL_DestroyWindow(screen->window);

    free(screen);

    SDL_Quit();
}
