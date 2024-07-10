#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "logging.h"

#define BOLD_RED "\x1b[1;31m"
#define BOLD_YELLOW "\x1b[0;33m"
#define BOLD_CYAN "\x1b[1;36m"
#define BOLD_PURPLE "\x1b[0;35m"
#define RESET "\x1b[0m"

bool debug_enable = false;

/**
 * @brief Print a message with a prefix and optionally embedded data like in `printf()`.
 *
 * @param message The message to print.
 * @param prefix The prefix to be applied to the message.
 * @param data The data to be embedded into the message.
 * @return
 */
void formatted_print(const char* message, const char* prefix, va_list data)
{
    fputs(prefix, stderr);
    vfprintf(stderr, message, data);
    fprintf(stderr, "\n");
}

/**
 * @brief Prints an error with embedded data like in `printf()`
 *
 * @param message The message to print.
 * @param ... The data to be embedded into the message.
 */
void error(const char* message, ...)
{
    va_list data;

    va_start(data, message);
    formatted_print(message, BOLD_RED "ERROR: " RESET, data);
    va_end(data);
}

/**
 * @brief Prints a warning with embedded data like in `printf()`
 *
 * @param message The message to print.
 * @param ... The data to be embedded into the message.
 */
void warning(const char* message, ...)
{
    va_list data;

    va_start(data, message);
    formatted_print(message, BOLD_YELLOW "WARNING: " RESET, data);
    va_end(data);
}

/**
 * @brief Prints an info message with embedded data like in `printf()`
 *
 * @param message The message to print.
 * @param ... The data to be embedded into the message.
 */
void info(const char* message, ...)
{
    va_list data;

    va_start(data, message);
    formatted_print(message, BOLD_CYAN "INFO: " RESET, data);
    va_end(data);
}

/**
 * @brief Prints an debug message with embedded data like in `printf()`
 *
 * @param message The message to print.
 * @param ... The data to be embedded into the message.
 */
void debug(const char* message, ...)
{
    if (!debug_enable) {
      return;
    }
    
    va_list data;

    va_start(data, message);
    formatted_print(message, BOLD_PURPLE "DEBUG: " RESET, data);
    va_end(data);
}
