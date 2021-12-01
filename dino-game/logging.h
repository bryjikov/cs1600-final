/*
   logging.h
   Macros for logging to the serial monitor.
*/

// Uncomment this to enable debug logging
#define DEBUG_LOGGING

#define LOG_BUFFER_SIZE 128
static char print_buffer[LOG_BUFFER_SIZE];
static char alt_buffer[LOG_BUFFER_SIZE];

// Print to the Serial using a format string.
// E.g. SERIAL_PRINTF("The number is %d\n", 10);
//
// See: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html for
// an explanation of the ## syntax used (allows arguments to be omitted
// if the string is not a format string).
#define SERIAL_PRINTF(format, ...) do { \
    snprintf(print_buffer, LOG_BUFFER_SIZE, format, ##__VA_ARGS__); \
    Serial.print(print_buffer); \
  } while (0)

// Print an error message (possibly using a format string),
// and then halt the system (loop forever).
#define HALT_WITH_ERROR(format, ...) do { \
    snprintf(alt_buffer, LOG_BUFFER_SIZE, format, ##__VA_ARGS__); \
    SERIAL_PRINTF("[ERROR] %s: %s\n", __func__, alt_buffer); \
    while (true); \
  } while (0)

// Print a debug message (possibly using a format string). This only
// does anything if DEBUG_LOGGING is defined; otherwise, it is a no-op.
#ifdef DEBUG_LOGGING
#define DEBUG(format, ...) do { \
    snprintf(alt_buffer, LOG_BUFFER_SIZE, format, ##__VA_ARGS__); \
    SERIAL_PRINTF("[DEBUG] %s: %s\n", __func__, alt_buffer); \
  } while (0)
#else
#define DEBUG(format, ...) do {} while (0)
#endif
