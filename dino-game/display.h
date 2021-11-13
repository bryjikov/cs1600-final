
// A person character
byte person[8] = {
  B00100,
  B01010,
  B00100,
  B11111,
  B00100,
  B00100,
  B01010,
  B10001,
};

//#define BIG_LCD

// Constants which define the dimensions of the LCD screen
#ifdef BIG_LCD

#define LCD_X_MIN 0
#define LCD_X_MAX 19
#define LCD_Y_MIN 0
#define LCD_Y_MAX 3

#else

#define LCD_X_MIN 0
#define LCD_X_MAX 15
#define LCD_Y_MIN 0
#define LCD_Y_MAX 1

#endif
