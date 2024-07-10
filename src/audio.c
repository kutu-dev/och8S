#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#include <math.h>
#include <stdint.h>

#include "audio.h"
#include "logging.h"

static constexpr double PI = 3.14159265358979323846;

static constexpr int SAMPLE_RATE = 44100;

/**
 * @brief Callback called when the SDL audio buffer needs to be filled. It expects a buffer with format `AUDIO_S16SYS`.
 *
 * @param user_data Expected to be a `uint32_t` that tracks the progression of the shound
 * @param raw_buffer The audio buffer to be filled.
 * @param bytes The size in bytes of the audio buffer.
 */
void audio_callback(void* user_data, Uint8* raw_buffer, int bytes)
{
    // The buffer is format with 16 bits
    Uint16* buffer = (Uint16*)raw_buffer;

    // 2 bytes per sample for AUDIO_S16SYS
    int buffer_length = bytes / 2;

    uint32_t* audio_sample_counter = (uint32_t*)user_data;

    constexpr uint16_t amplitude = 2000;
    constexpr uint16_t frequency = 440;

    for (int i = 0; i < buffer_length; i++, (*audio_sample_counter)++) {
        // Time pass inside the sample from 0 to 1.
        double time = (double)(*audio_sample_counter) / (double)SAMPLE_RATE;

        // Sinusoidal equation
        buffer[i] = (Uint16)(amplitude * sin(2 * PI * frequency * time));
    }
}

/**
 * @brief Setup the SDL audio to generate a beep sound when `SDL_Pause(0)` is called. At the end of the execution `SDL_CloseAudio()`should be called.
 *
 * @param audio_sample_counter Variable were the progression of the sound will be stored, it should last up to the moment when the `SDL_CloseAudio()` function is called. It's recommended to be put to 0 when calling `SDL_Pause(1)` to mitigate random audio cracking.
 */
uint8_t setup_audio(uint32_t* audio_sample_counter)
{
    SDL_AudioSpec desired;

    desired.freq = SAMPLE_RATE;
    desired.format = AUDIO_S16SYS;
    desired.channels = 1;
    desired.samples = 2048;
    desired.callback = audio_callback;
    desired.userdata = audio_sample_counter;

    SDL_AudioSpec obtained;

    if (SDL_OpenAudio(&desired, &obtained) != 0) {
        error("Failed to open audio: %s", SDL_GetError());
        return 1;
    }

    if (desired.format != obtained.format) {
        error("Failed to get the desired AudioSpec format");
        return 2;
    }

    return 0;
}
