#include <LiquidCrystal.h>
#include <LinkedPointerList.h>
#include "obstacles.h"
#include "timer.h"
#include "display.h"
#include "led.h"

#define SERIAL_PRINTF_BUF_SIZE 512
char SERIAL_PRINTF_BUF[SERIAL_PRINTF_BUF_SIZE];

// This macro allows us to print to the Serial using format strings.
// E.g. serial_printf("The number is %d\n", 10);
#define serial_printf(format, args...) do { \
    snprintf(SERIAL_PRINTF_BUF, SERIAL_PRINTF_BUF_SIZE, format, args); \
    Serial.print(SERIAL_PRINTF_BUF); \
  } while (0)

#define joyX A0
#define joyY A1
int joystickPosX = 0;
int joystickPosY = 0;
int joystickPrevPosX = 0;
int joystickPrevPosY = 0;
int joystickInitialPosX = 0;
int joystickInitialPosY = 0;

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

// Prints an error message and halts the system
void error(String msg) {
  Serial.println(msg);
  while (true);
}

void obstacle_move_handler(void)
{
  // TODO: Assume for now we're just moving left (this will depend on FSM)
  move_obstacles(all_obstacles, LEFT);
  remove_out_of_bounds(all_obstacles);

  // Spawn a new obstacle with probability 1/8
  if (rand() % 8 == 0) {
    spawn_random_obstacle(all_obstacles, LEFT);
  }
}

void obstacle_speed_up_handler(void)
{
  job_t *job = get_job(MOVE_OBSTACLES);
  // Subtracting 1 is 50ms less because this is in terms of the DRIVER_INTERVAL (which is 50ms)
  update_interval_multiple(MOVE_OBSTACLES, job->interval_multiple - 1);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  initialize_lcd();

  pinMode(buttonPin, INPUT);
  pinMode(LED_PIN, OUTPUT);
  attachInterrupt(joyY, jumpUpInterrupt, RISING);
  //attachInterrupt(joyY, jumpDownInterrupt, FALLING);

  //setup_watchdog();

  // Configure the timer driver to run every DRIVER_INTERVAL ms
  tcConfigure(DRIVER_INTERVAL);

  all_obstacles = new LinkedPointerList<obstacle_t>();
  all_jobs = new LinkedPointerList<job_t>();

  register_job(MOVE_OBSTACLES, &obstacle_move_handler, 250);
  register_job(SPEED_UP_OBSTACLES, &obstacle_speed_up_handler, 1000);

  joystickInitialPosX = analogRead(joyX);
  joystickInitialPosY = analogRead(joyY);
}

void loop()
{
  update_joystick();
  //pet_watchdog();
  update_player_state(millis());
  display_player(8, positionY);
  //display_obstacles(all_obstacles);
  updateLED();
}

void update_joystick() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  // map the joysticks value (0, 1023) to (-1, 1) where it is only 0 if at center position (500, 515)
  joystickPosX = xValue >= joystickInitialPosX - 15 && xValue <= joystickInitialPosX + 15 ? 0 : xValue > joystickInitialPosX + 15 ? 1 : -1;
  joystickPosY = yValue >= joystickInitialPosY - 15 && yValue <= joystickInitialPosY + 15 ? 0 : yValue > joystickInitialPosY + 15 ? 1 : -1;
  if(joystickPosX != joystickPrevPosX || joystickPosY != joystickPrevPosY) {
    joystick_position_changed();
  }
  joystickPrevPosX = joystickPosX;
  joystickPrevPosY = joystickPosY;
}

void joystick_position_changed() {
  clear();
  positionY = constrain(positionY + joystickPosY, minY, maxY);
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
