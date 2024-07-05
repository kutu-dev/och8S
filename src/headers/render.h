#ifndef OCH8S_RENDER_H
#define OCH8S_RENDER_H

#include <stdbool.h>
#include <SDL2/SDL.h>

typedef struct Screen {
  SDL_Window* window;
  SDL_Renderer* renderer;
} Screen;

typedef struct ScreenBuffer {
  size_t screen_height;
  size_t screen_width;
  bool **buffer;
} ScreenBuffer;

void draw_screen_buffer(ScreenBuffer* screen_buffer, Screen *screen);

Screen* create_screen(void);

ScreenBuffer* create_screen_buffer(size_t screen_height, size_t screen_width);

void delete_screen_buffer(ScreenBuffer* screen_buffer);

void delete_screen(Screen* screen);

#endif
