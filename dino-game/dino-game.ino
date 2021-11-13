#include <LiquidCrystal.h>
#include "obstacles.h"
#include "display.h"

#define joyX A0
#define joyY A1
int joystickPosX = 0;
int joystickPosY = 0;
int joystickPrevPosX = 0;
int joystickPrevPosY = 0;

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int buttonPin = 8;
int minY = 0;
int maxY = 1;
int positionY = 1;
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

obstacle_t o;

//LinkedList<obstacle_t> *all_obstacles;

// Prints an error message and halts the system
void error(String msg) {
  Serial.println(msg);
  while (true);
}

void setup()
{
  Serial.begin(9600);
  lcd.createChar(0, person);
  lcd.begin(16, 2);
  pinMode(buttonPin, INPUT);
  //attachInterrupt(joyY, jumpUpInterrupt, RISING);
  //attachInterrupt(joyY, jumpDownInterrupt, FALLING);
  tcConfigure(sampleRate); // configure the timer to run at <sampleRate>Hertz
  pinMode(ledPin, OUTPUT);
  o = {15, 1};
  //all_obstacles = new LinkedList<obstacle_t>();
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
  update_joystick();
  //pet_watchdog();
  update_player_state(millis());
  display_player(8, positionY);
  display_obstacle(o);
  updateLED();
}

void update_joystick() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  // map the joysticks value (0, 1023) to (-1, 1) where it is only 0 if at center position (500, 515)
  joystickPosX = xValue >= 500 && xValue <= 515 ? 0 : xValue > 515 ? 1 : -1;
  joystickPosY = yValue >= 500 && yValue <= 515 ? 0 : yValue > 515 ? 1 : -1;
  if(joystickPosX != joystickPrevPosX || joystickPrevPosY != joystickPrevPosY) {
    joystick_position_changed();
  }
  joystickPrevPosX = joystickPosX;
  joystickPrevPosY = joystickPosY;
}

void joystick_position_changed() {
  positionY = constrain(positionY + joystickPosY, minY, maxY);
  Serial.println(joystickPosX);
  Serial.println(joystickPosY);
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
