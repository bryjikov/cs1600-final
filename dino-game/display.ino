/*
   display.ino
   Functions for interacting with the LCD display
   and producing the game's graphics.
*/

#define BLACK_SQUARE 255

/*
   Displays the player at the given coordinate of the LCD display.
*/
void display_player(byte x, byte y)
{
  lcd.setCursor(x, y);
  lcd.write(byte(0));
}

/*
   Displays the given obstacle to the LCD display.
*/
void display_obstacle(obstacle_t *obs)
{
  lcd.setCursor(obs->x, obs->y);
  lcd.write(byte(BLACK_SQUARE));
}
