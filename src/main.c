#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "render.h"

int main() {
  Screen *screen = create_screen();

  if (screen == NULL) {
    return 1;
  }

  ScreenBuffer *screen_buffer = create_screen_buffer(32, 64);
  (screen_buffer->buffer)[0][32] = true;

  uint8_t memory[4098] = {0};

  uint8_t font[16 * 5] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
  };

  // Copy the font to the memory at `050` (80 bits)
  memcpy(memory + 80 / sizeof(memory[0]), font, sizeof(font));

  FILE *rom = fopen("rom.ch8", "rb");

  // Get the size of the file in bytes
  fseek(rom, 0, SEEK_END);
  long size = ftell(rom);
  rewind(rom);

  fread(memory + 512 / sizeof(memory[0]), sizeof(memory[0]), size, rom);

  fclose(rom);

  FILE *test = fopen("test.bin", "wb");
  fwrite(memory, sizeof(memory), 1, test);

  fclose(test);

  uint16_t index_register = 0;

  uint16_t pc = 512 / sizeof(memory[0]);

  draw_screen_buffer(screen_buffer, screen);
  while (1) {
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_QUIT) {
      break;
    }
    
    printf("%02x%02x at %u\n", memory[pc], memory[pc + 1], pc);

    pc += 2;
  }

  delete_screen_buffer(screen_buffer);
  delete_screen(screen);
}
