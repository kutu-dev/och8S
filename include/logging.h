#ifndef OCH8S_LOGGING_H
#define OCH8S_LOGGING_H

#include <stdint.h>

void error(const char* message, ...);
void warning(const char* message, ...);
void info(const char* message, ...);
void debug(const char* message, ...);

#endif
