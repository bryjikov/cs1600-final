#include <LiquidCrystal.h>
#include "watchdog.h"

#define joyX A0
#define joyY A1
int joystickPosX = 0;
int joystickPosY = 0;
int joystickPrevPosX = 0;
int joystickPrevPosY = 0;

byte arrows[8][8] = {{B00100, B01110, B11111, B00000, B00000, B00000, B00000, B00000},
  {B00000, B11111, B01110, B00100, B00000, B00000, B00000, B00000},
  {B00100, B01100, B11100, B01100, B00100, B00000, B00000, B00000},
  {B00100, B00110, B00111, B00110, B00100, B00000, B00000, B00000},
  {B00000, B00000, B00000, B00000, B00100, B01110, B11111, B00000},
  {B00000, B00000, B00000, B00000, B00000, B11111, B01110, B00100},
  {B00000, B00000, B00000, B00100, B01100, B11100, B01100, B00100},
  {B00000, B00000, B00000, B00100, B00110, B00111, B00110, B00100}
};

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int buttonPin = 8;
int positionY = 3;
uint32_t sampleRate = 1000; // sample rate in milliseconds, determines how often TC5_Handler is called

#define PWM_MIN 0
#define PWM_MAX 255

int ledBrightness = 0;
bool brightnessAscending = true;

/* FLAGS */
volatile bool jump_start_flag = false;
volatile bool jump_end_flag = false;
/* FLAGS */

typedef enum
{
  INACTIVE = 0,
  RUNNING = 1,
  JUMPING = 2,
  /* .. continue ... */
} playerState;

playerState PLAYER_STATE = RUNNING;

const int ledPin = 6;

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  pinMode(buttonPin, INPUT);
  attachInterrupt(joyY, jumpChangeInterrupt, FALLING);
  //attachInterrupt(joyY, jumpUpInterrupt, RISING);
  //attachInterrupt(joyX, jumpDownInterrupt, RISING);
  //tcConfigure(sampleRate); // configure the timer to run at <sampleRate>Hertz
  pinMode(ledPin, OUTPUT);
}

void display_cursor(byte x, byte y)
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

/**
   Update the pulsing LED's brightness to cycle between the
   min and max PWM values, so the LED pulsates constantly.
*/
void updateLED(void)
{
  // set the LED at current brightness
  analogWrite(ledPin, ledBrightness);

  // increase/decrease brightness
  if (brightnessAscending) {
    ledBrightness++;
  } else {
    ledBrightness--;
  }

  // change direction of pulsing if at lower/upper ends
  if (ledBrightness == PWM_MIN || ledBrightness == PWM_MAX) {
    brightnessAscending = !brightnessAscending;
  }
}

void loop()
{
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR(165);
  update_joystick();
  update_joystick();
  pet_watchdog();
  update_player_state(millis());
  display_player(8, positionY);
  updateLED();
}

void update_joystick() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  // map the joysticks value (0, 1023) to (-1, 1) where it is only 0 if at center position (511)
  joystickPosX = xValue = 511 ? 0 : xValue > 511 ? 1 : -1;
  joystickPosY = yValue = 511 ? 0 : yValue > 511 ? 1 : -1;
  if(joystickPosX != joystickPrevPosX || joystickPrevPosY != joystickPrevPosY) {
    /* joystick position changed */
  }
  joystickPrevPosX = joystickPosX;
  joystickPrevPosY = joystickPosY;
}

void update_player_state(long mils)
{
  playerState NEXT_STATE = PLAYER_STATE;
  switch (PLAYER_STATE)
  {
    case INACTIVE:
      NEXT_STATE = player_state_inactive();
      break;
    case RUNNING:
      NEXT_STATE = player_state_running();
      break;
    case JUMPING:
      NEXT_STATE = player_state_jumping();
      break;
    default:
      break;
  }
  PLAYER_STATE = NEXT_STATE;
}

playerState player_state_inactive()
{
  return INACTIVE;
}

playerState player_state_running()
{
  if (jump_start_flag)
  {
    positionY = 2;
    //tcStartCounter();
    jump_start_flag = false;
    //Serial.println("jump_start_flag was true");
    return JUMPING;
  }
  else {
    return RUNNING;
  }
}

playerState player_state_jumping()
{
  if (jump_end_flag)
  {
    positionY = 3;
    jump_end_flag = false;
    //Serial.println("jump_end_flag was true");
    return RUNNING;
  }
  else {
    return JUMPING;
  }
}

void jumpChangeInterrupt() {
  //Serial.println("In change");
  int yVal = analogRead(joyY);
  //Serial.print("yval:");
  //Serial.println(yVal);
  if (yVal < 10) {
    //Serial.println("yVal < 10");
    jump_start_flag = true;
  }
  if (yVal > 1013) {
    //Serial.println("yVal > 1013");
    jump_end_flag = true;
  }
}

void jumpUpInterrupt()
{
  //Serial.println("jumping");
  jump_start_flag = true;
}

void jumpDownInterrupt()
{
  jump_end_flag = true;
}
