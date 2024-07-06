#include <SDL2/SDL.h>

/**
 * @brief Maps a CHIP-8 key (the index of the array) with a SDL scancode (the value of the array)
 */
const uint8_t chip8_key_to_sdl_scancode[] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, // 1
    SDL_SCANCODE_2, // 2
    SDL_SCANCODE_3, // 3
    SDL_SCANCODE_Q, // 4
    SDL_SCANCODE_W, // 5
    SDL_SCANCODE_E, // 6
    SDL_SCANCODE_A, // 7
    SDL_SCANCODE_S, // 8
    SDL_SCANCODE_D, // 9
    SDL_SCANCODE_Z, // A
    SDL_SCANCODE_C, // B 
    SDL_SCANCODE_4, // C
    SDL_SCANCODE_R, // D
    SDL_SCANCODE_F, // E
    SDL_SCANCODE_V // F
};

/**
 * @brief Get the mapped CHIP-8 key to the given SDL scancode
 *
 * @param scancode The scancode to be mapped
 * @return The mapped key or -1 if it is out of the CHIP-8 keypad range
 */
uint8_t sdl_scancode_to_chip8_key(uint8_t scancode)
{
    switch (scancode) {
    case SDL_SCANCODE_1:
        return 1;
        break;

    case SDL_SCANCODE_2:
        return 2;
        break;

    case SDL_SCANCODE_3:
        return 3;
        break;

    case SDL_SCANCODE_4:
        return 12;
        break;

    case SDL_SCANCODE_Q:
        return 4;
        break;

    case SDL_SCANCODE_W:
        return 5;
        break;

    case SDL_SCANCODE_E:
        return 6;
        break;

    case SDL_SCANCODE_R:
        return 13;
        break;

    case SDL_SCANCODE_A:
        return 7;
        break;

    case SDL_SCANCODE_S:
        return 8;
        break;

    case SDL_SCANCODE_D:
        return 9;
        break;

    case SDL_SCANCODE_F:
        return 14;
        break;

    case SDL_SCANCODE_Z:
        return 10;
        break;

    case SDL_SCANCODE_X:
        return 0;
        break;

    case SDL_SCANCODE_C:
        return 11;
        break;

    case SDL_SCANCODE_V:
        return 15;
        break;

  default:
      return -1;
      break;
    }
}
