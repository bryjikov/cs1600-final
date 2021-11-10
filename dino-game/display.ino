/*
   display.ino
   Functions for interacting with the LCD display
   and producing the game's graphics.
*/

byte arrows[8][8] = {
  {B00100, B01110, B11111, B00000, B00000, B00000, B00000, B00000},
  {B00000, B11111, B01110, B00100, B00000, B00000, B00000, B00000},
  {B00100, B01100, B11100, B01100, B00100, B00000, B00000, B00000},
  {B00100, B00110, B00111, B00110, B00100, B00000, B00000, B00000},
  {B00000, B00000, B00000, B00000, B00100, B01110, B11111, B00000},
  {B00000, B00000, B00000, B00000, B00000, B11111, B01110, B00100},
  {B00000, B00000, B00000, B00100, B01100, B11100, B01100, B00100},
  {B00000, B00000, B00000, B00100, B00110, B00111, B00110, B00100}
};

/*
   Displays the player at the given coordinate of the LCD display.
*/
void display_player(byte x, byte y)
{
  // A hack, because enum "orientation" defines values from 0 to 3 and we defined arrow indexing in this way
  // we are converting LCD y-coordinates (0 and 1) to game y-coordinates (0, 1, 2, and 3)
  int arrow_ind = 4 * (y % 2);
  // only 8 custom characters are allowed to be stored at once, so we have to swap them out sometimes
  lcd.createChar(arrow_ind, arrows[arrow_ind]);
  lcd.clear();
  lcd.setCursor(x, y / 2);
  lcd.write(byte(arrow_ind));
}

/*
   Displays the given obstacle to the LCD display.
*/
void display_obstacle(obstacle_t obs)
{
  // TODO: implement this
}
