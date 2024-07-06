#ifndef OCH8S_KEYS_H
#define OCH8S_KEYS_H

#include <stdint.h>

extern const uint8_t chip8_key_to_sdl_scancode[];

uint8_t sdl_scancode_to_chip8_key(uint8_t scancode);

#endif
