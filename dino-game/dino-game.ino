#include <LiquidCrystal.h>

byte arrows[8][8] = {{ B00100, B01110, B11111, B00000, B00000, B00000, B00000, B00000 },
                     { B00000, B11111, B01110, B00100, B00000, B00000, B00000, B00000 },
                     { B00100, B01100, B11100, B01100, B00100, B00000, B00000, B00000 },
                     { B00100, B00110, B00111, B00110, B00100, B00000, B00000, B00000 },
                     { B00000, B00000, B00000, B00000, B00100, B01110, B11111, B00000 },
                     { B00000, B00000, B00000, B00000, B00000, B11111, B01110, B00100 },
                     { B00000, B00000, B00000, B00100, B01100, B11100, B01100, B00100 },
                     { B00000, B00000, B00000, B00100, B00110, B00111, B00110, B00100 }};

void setup() {
  Serial.begin(9600);
  while (!Serial);

  lcd.begin(16,2);
}

void display_cursor(byte x, byte y) {
  // A hack, because enum "orientation" defines values from 0 to 3 and we defined arrow indexing in this way
  // we are converting LCD y-coordinates (0 and 1) to game y-coordinates (0, 1, 2, and 3)
  int arrow_ind = 4 * (y%2);
  // only 8 custom characters are allowed to be stored at once, so we have to swap them out sometimes
  lcd.createChar(arrow_ind, arrows[arrow_ind]);
  lcd.clear();
  lcd.setCursor(x, y/2);
  lcd.write(byte(arrow_ind));
}

void loop(){
    display_cursor(8, 2);
}