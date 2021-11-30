/*
   display.ino
   Functions for interacting with the LCD display
   and producing the game's graphics.
*/

#define PLAYER_CUSTOM_CHAR 0
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

/*
   Sets up the LCD screen with appropriate dimensions and creates
   a custom character for displaying the player.
*/
void initialize_lcd(void)
{
  lcd.init();
  lcd.backlight();
  lcd.createChar(PLAYER_CUSTOM_CHAR, person);
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
  lcd.write(byte(BLACK_SQUARE));
}

/*
   Displays a screen indicating that the game has been lost.
*/
void display_game_over(unsigned long total_time){
  // TODO: display game over screen here
  clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!");
  lcd.setCursor(0,1);
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
