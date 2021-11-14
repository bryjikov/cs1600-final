/*
 * display.h
 * Constants for interacting with the LCD.
 */

// Uncomment this for big LCD.
//#define BIG_LCD

// Constants which define the dimensions of the LCD screen
#ifdef BIG_LCD

#define LCD_X_DIM 20
#define LCD_Y_DIM 4

#define LCD_X_MIN 0
#define LCD_X_MAX 19
#define LCD_Y_MIN 0
#define LCD_Y_MAX 3

#else

#define LCD_X_DIM 16
#define LCD_Y_DIM 2

#define LCD_X_MIN 0
#define LCD_X_MAX 15
#define LCD_Y_MIN 0
#define LCD_Y_MAX 1

#endif
