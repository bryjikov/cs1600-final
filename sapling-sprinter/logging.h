/*
   logging.h
   Macros for logging to the serial monitor.
*/

#include <stdarg.h>

// Uncomment this to enable debug logging
#define DEBUG_LOGGING

#define LOG_BUFFER_SIZE 128

// Write a format string to a buffer.
#define FORMAT_TO_BUFFER(buf, amount, format) do { \
    va_list ap; \
    va_start(ap, format); \
    vsnprintf(buf, amount, format, ap); \
    va_end(ap); \
  } while (0)

/*
   Prints to the serial monitor with a format string.
*/
void serial_printf(char *format, ...)
{
  char buf[LOG_BUFFER_SIZE];
  FORMAT_TO_BUFFER(buf, LOG_BUFFER_SIZE, format);
  Serial.print(buf);
}

/**
 * Prints a string literal to Serial, but stores the string
 * in flash memory to save RAM.
 * 
 * More info: https://playground.arduino.cc/Learning/Memory/
 */
#define PRINTLN_FLASH(str) Serial.println(F(str))

/*
   Macro wrapper around halt_with_error_underlying so we can call it
   with the appropriate value of __func__
*/
#define halt_with_error(format, ...) \
  halt_with_error_underlying(__func__, format, ##__VA_ARGS__)

/*
   Print an error message (possibly using a format string),
   and then halt the system (loop forever).
*/
void halt_with_error_underlying(char *func, char *format, ...)
{
  char buf[LOG_BUFFER_SIZE];
  FORMAT_TO_BUFFER(buf, LOG_BUFFER_SIZE, format);
  serial_printf("[ERROR] %s: %s\n", func, buf);
  while (true);
}

/*
   Macro wrapper around debug_underlying so we can call it
   with the appropriate value of __func__
*/
#define debug(format, ...) \
  debug_underlying(__func__, format, ##__VA_ARGS__)

// Print a debug message (possibly using a format string). This only
// does anything if DEBUG_LOGGING is defined; otherwise, it is a no-op.
#ifdef DEBUG_LOGGING
void debug_underlying(char *func, char *format, ...)
{
  char buf[LOG_BUFFER_SIZE];
  FORMAT_TO_BUFFER(buf, LOG_BUFFER_SIZE, format);
  serial_printf("[DEBUG] %s: %s\n", func, buf);
}
#else
void debug_underlying(char *func, char *format, ...) {}
#endif
