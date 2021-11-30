/*
   display.ino
   Functions for interacting with the LCD display
   and producing the game's graphics.
*/

#define PLAYER_CUSTOM_CHAR 0
#define OBSTACLE_CUSTOM_CHAR 1
#define BLACK_SQUARE 255

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

// An obstacle looks like a Christmas tree
byte obstacle[8] = {
  B00100,
  B00100,
  B01110,
  B01110,
  B11111,
  B01110,
  B11111,
  B00100,
};

/*
   Sets up the LCD screen with appropriate dimensions and creates
   a custom character for displaying the player.
*/
void initialize_lcd(void)
{
#ifdef BIG_LCD
  lcd.init();
  lcd.backlight();
  lcd.createChar(PLAYER_CUSTOM_CHAR, person);
  lcd.createChar(OBSTACLE_CUSTOM_CHAR, obstacle);
#else
  lcd.createChar(PLAYER_CUSTOM_CHAR, person);
  lcd.createChar(OBSTACLE_CUSTOM_CHAR, obstacle);
  lcd.begin(LCD_X_DIM, LCD_Y_DIM);
#endif
}

/*
   Displays the player at the given coordinate of the LCD display.
*/
void display_player(byte x, byte y)
{
  lcd.setCursor(x, y);
  lcd.write(byte(PLAYER_CUSTOM_CHAR));
}

/*
   Displays the given obstacle to the LCD display.
*/
void display_obstacle(obstacle_t *obs)
{
  lcd.setCursor(obs->x, obs->y);
  lcd.write(byte(OBSTACLE_CUSTOM_CHAR));
}

/*
   Displays a screen which indicates that the game is about to begin.
*/
void display_setup(void)
{
  clear();
  lcd.setCursor(LCD_TEXT_CENTER_X, LCD_TEXT_CENTER_Y);
  lcd.print("Ready?  :O");
}

/*
   Displays a screen indicating that the game has been lost.
*/
void display_game_over(unsigned long total_time)
{
  clear();
  lcd.setCursor(LCD_TEXT_CENTER_X, LCD_TEXT_CENTER_Y);
  lcd.print("Game Over!");
  lcd.setCursor(LCD_TEXT_CENTER_X-1, LCD_TEXT_CENTER_Y + 1);
  lcd.print("Score: ");
  lcd.print(total_time);
}

/*
   Clears LCD display.
*/
void clear()
{
  lcd.clear();
}
