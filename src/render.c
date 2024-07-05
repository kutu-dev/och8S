#include <SDL2/SDL.h>
#include <stdbool.h>

#include "render.h"

typedef struct Screen Screen;
typedef struct ScreenBuffer ScreenBuffer;

void log_error(char *message) {
  SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, message, SDL_GetError());
}

void clear_screen(SDL_Renderer *renderer) {
  if (SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF) != 0) {
    log_error("Couldn't set the draw color: %s");
  }

  SDL_RenderClear(renderer);
}

void draw_pixel(int x, int y, SDL_Renderer *renderer) {
  SDL_Rect pixel;

  pixel.x = x;
  pixel.y = y;
  pixel.w = 1;
  pixel.h = 1;

  if (SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF) != 0) {
    log_error("Couldn't set the draw color: %s");
  }

  if (SDL_RenderDrawRect(renderer, &pixel) != 0) {
    log_error("Couldn't create pixel: %s");
  }
}

void draw_screen_buffer(ScreenBuffer *screen_buffer, Screen *screen) {
  clear_screen(screen->renderer);

  for (size_t y = 0; y <= screen_buffer->screen_height; y++) {
    for (size_t x = 0; x <= screen_buffer->screen_width; x++) {
      if (!screen_buffer->buffer[y][x]) {
        continue;
      }

      draw_pixel(x, y, screen->renderer);
    }
  }

  SDL_RenderPresent(screen->renderer);
}

Screen *create_screen() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    log_error("error initializing SDL: %s");
    return NULL;
  }

  SDL_Window *window =
      SDL_CreateWindow("och8S", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, 64 * 8, 32 * 8, 0);
  if (window == NULL) {
    log_error("Couldn't create window: %s");
    return NULL;
  }

  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == NULL) {
    log_error("Couldn't create renderer: %s");
    return NULL;
  }

  if (SDL_RenderSetLogicalSize(renderer, 64, 32)) {
    log_error("Couldn't set the render logical size");
    return NULL;
  }

  Screen *screen = malloc(sizeof(Screen));

  screen->window = window;
  screen->renderer = renderer;

  return screen;
}

ScreenBuffer *create_screen_buffer(size_t screen_height, size_t screen_width) {
  ScreenBuffer *screen_buffer = malloc(sizeof(ScreenBuffer));

  screen_buffer->screen_height = screen_height;
  screen_buffer->screen_width = screen_width;

  screen_buffer->buffer = malloc(sizeof(bool *) * screen_height);

  for (size_t i = 0; i <= screen_height; i++) {
    screen_buffer->buffer[i] = malloc(sizeof(bool) * 64);

    for (size_t j = 0; j <= screen_width; j++) {
      screen_buffer->buffer[i][j] = false;
    }
  }

  return screen_buffer;
}

void delete_screen_buffer(ScreenBuffer *screen_buffer) {
  for (size_t y = 0; y <= screen_buffer->screen_height; y++) {
    free(screen_buffer->buffer[y]);
  }

  free(screen_buffer->buffer);
  free(screen_buffer);
}

void delete_screen(Screen *screen) {
  SDL_DestroyRenderer(screen->renderer);
  SDL_DestroyWindow(screen->window);

  free(screen);

  SDL_Quit();
}
