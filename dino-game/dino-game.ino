#include <LinkedPointerList.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include "obstacles.h"
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

/* Joystick Variables */
#define joyX A0
#define joyY A1
#define BUTTON_PIN 8
int joystickPosX = 0;
int joystickPosY = 0;
int joystickPrevPosX = 0;
int joystickPrevPosY = 0;
int joystickInitialPosX = 0;
int joystickInitialPosY = 0;

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;

#ifdef BIG_LCD
LiquidCrystal_I2C lcd(0x27, LCD_X_DIM, LCD_Y_DIM);
#else
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#endif


/* FSM Constants and Variables */
#define PRE_DIR_CHG_DURATION              2000    /* How long to remain in PDC state (ms) */
#define DIR_CHG_INTERVAL                  10000   /* How often (ms) a direction change potentially happens */
#define SPEED_UP_INTERVAL                 5000    /* How often (ms) obstacles are sped up */
#define INIT_OBSTACLE_MOVE_INTERVAL       1000    /* How often obstacles are moved, initially */
#define OBSTACLE_MOVE_INTERVAL_DECREASE   50      /* By how much (ms) does the obstacle move interval decrease */

volatile bool game_over_flag;                     /* Set when the player has collided with an obstacle */
volatile bool pre_direction_change_flag;          /* Set when a direction change is upcoming and user should be warned */
volatile unsigned long time_entered_pdc;          /* Time (ms) at which the pre direction change state was entered */
volatile bool restart_flag;                       /* Set when the game should start over */
volatile unsigned obstacle_move_interval;         /* How often (ms) obstacles move */
volatile unsigned long time_last_obstacle_move;   /* Time (ms) of last obstacle movement */
volatile unsigned long time_last_dir_chg;         /* Time (ms) of last direction change event */
volatile unsigned long time_last_speed_up;        /* Time (ms) that obstacles were last sped up */
volatile direction_t obstacle_direction;          /* Direction of movement for obstacles */
volatile int player_x;                            /* Current X position of the player */
volatile int player_y;                            /* Current Y position of the player */
volatile bool moved;                              /* Set if either the player or obstacles moved since the last display */
volatile unsigned long start_time;                /* The time (in milis) when the player begins */
volatile unsigned long duration;                  /* The total time that the game lasted */
LinkedPointerList<obstacle_t> *all_obstacles;     /* List containing all currently active obstacles */

typedef enum
{
  /* All FSM variables should be initialized */
  SETUP,
  /* Normal gameplay, obstacles are moving and player is responding. */
  NORMAL,
  /* Temporary period during which LED pulsates to warn of an upcoming direction change. */
  PRE_DIRECTION_CHANGE,
  /* Player has collided with obstacle, game is over. */
  GAME_OVER,
} state_t;

state_t current_state;

// Prints an error message and halts the system
void error(String msg) {
  Serial.println(msg);
  while (true);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("setting up game...");
  //  while (!Serial);

  srand(time(NULL));

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonPressInterrupt, RISING);

  current_state = SETUP;

  all_obstacles = new LinkedPointerList<obstacle_t>();

  initialize_lcd();
  initialize_fsm();

  timer_setup();

  joystickInitialPosX = analogRead(joyX);
  joystickInitialPosY = analogRead(joyY);

  stop_watchdog();
  setup_watchdog();
}

void loop()
{
  pet_watchdog();
  current_state = update_game_state(millis());
}

/*
   Sets all FSM variables back to their initial states.
*/
void initialize_fsm(void) {
  game_over_flag = false;
  pre_direction_change_flag = false;
  time_entered_pdc = 0;
  restart_flag = false;
  obstacle_move_interval = INIT_OBSTACLE_MOVE_INTERVAL;
  time_last_obstacle_move = 0;
  time_last_dir_chg = 0;
  time_last_speed_up = 0;
  start_time = 0;
  duration = 0;
  player_x = LCD_X_DIM / 2; // Start the player at the bottom and middle of the screen
  player_y = LCD_Y_MAX;
  obstacle_direction = LEFT;
  moved = true;
  all_obstacles->clear();
}

/*
  Update Joystick position and map the joysticks value (0, 1023) to (-1, 1) where it is only 0 if at center position (500, 515)
*/
void update_joystick() {
  int xValue = analogRead(joyX);
  int yValue = analogRead(joyY);
  joystickPosX = xValue >= joystickInitialPosX - 15 && xValue <= joystickInitialPosX + 15 ? 0 : xValue > joystickInitialPosX + 15 ? 1 : -1;
  joystickPosY = yValue >= joystickInitialPosY - 15 && yValue <= joystickInitialPosY + 15 ? 0 : yValue > joystickInitialPosY + 15 ? 1 : -1;
  if (joystickPosX != joystickPrevPosX || joystickPosY != joystickPrevPosY) {
    joystick_position_changed();
  }
  joystickPrevPosX = joystickPosX;
  joystickPrevPosY = joystickPosY;
}

void joystick_position_changed() {
  player_x = constrain(player_x + joystickPosX, LCD_X_MIN, LCD_X_MAX);
  player_y = constrain(player_y + joystickPosY, LCD_Y_MIN, LCD_Y_MAX);
  moved = true;
}

/*
   Checks any conditions and updates any state variables for normal gameplay
   (states NORMAL and PRE_DIRECTION_CHANGE). This involves detecting collisions,
   moving obstacles, speeding up obstacles, and causing direction changes.
*/
void update_for_normal_gameplay(unsigned long mils)
{
  // If it has been long enough since last obstacle move
  if (mils - time_last_obstacle_move > obstacle_move_interval) {
    // Move obstacles in the current direction and eliminate any that are out of bounds
    move_obstacles(all_obstacles, obstacle_direction);
    remove_out_of_bounds(all_obstacles);

    // Spawn a new obstacle with probability 1/4
    if (rand() % 4 == 0) {
      spawn_random_obstacle(all_obstacles, obstacle_direction);
    }

    time_last_obstacle_move = mils;
    moved = true;
  }
  // If it has been long enough since last obstacle speed-up
  if (mils - time_last_speed_up > SPEED_UP_INTERVAL) {
    obstacle_move_interval -= OBSTACLE_MOVE_INTERVAL_DECREASE;
    time_last_speed_up = mils;
  }
  // If it has been long enough since last direction change event
  if (mils - time_last_dir_chg > DIR_CHG_INTERVAL) {
    // With probability 1/4, trigger a direction change
    if (rand() % 4 == 0) {
      pre_direction_change_flag = true;
      time_entered_pdc = millis();
    }
    time_last_dir_chg = mils;
  }
  // If a collision is detected given the current position of the player
  // and configuration of the obstacles, indicate that it is game over.
  if (collision_detected(all_obstacles, player_x, player_y)) {
    game_over_flag = true;
    duration = millis() - start_time;
  }
  update_joystick();

  if (moved) {
    clear();
    display_player(player_x, player_y);
    display_obstacles(all_obstacles);
    moved = false;
  }
}

/*
   Updates the FSM, given the current time in milliseconds.

   TODO: label all transitions according to an FSM diagram, which we
   also need to update.
*/
state_t update_game_state(unsigned long mils)
{
  // By default, remain in the current state
  state_t next_state = current_state;

  //  noInterrupts(); // Mask interrupts to protect access to globals

  switch (current_state) {
    case SETUP:
      initialize_fsm();
      Serial.println("TRANSITIONING TO NORMAL");
      next_state = NORMAL;
      start_time = millis();
      break;

    case GAME_OVER:
      display_game_over();
      if (restart_flag) {
        Serial.println("TRANSITIONING TO SETUP");
        next_state = SETUP;
        restart_flag = false;
      }
      break;

    case NORMAL:
      update_for_normal_gameplay(mils);

      // Collision has occurred; game over.
      if (game_over_flag) {
        Serial.println("TRANSITIONING TO GAME OVER");
        next_state = GAME_OVER;
        game_over_flag = false;
        // It is time for a direction change
      } else if (pre_direction_change_flag) {
        Serial.println("TRANSITIONING TO PRE DIRECTION");
        next_state = PRE_DIRECTION_CHANGE;
        pre_direction_change_flag = false;
      }
      break;

    case PRE_DIRECTION_CHANGE:
      update_for_normal_gameplay(mils);

      // It's possible a collision occurs during pre-direction change
      if (game_over_flag) {
        Serial.println("TRANSITIONING TO GAME OVER");
        next_state = GAME_OVER;
        game_over_flag = false;
      }
      // If it has been long enough since the pre direction change state was
      // entered, flip the direction of obstacle movement and transition to NORMAL.
      else if (mils - time_entered_pdc >= PRE_DIR_CHG_DURATION) {
        Serial.println("TRANSITIONING TO NORMAL");
        obstacle_direction = invert_direction(obstacle_direction);
        next_state = NORMAL;
      }
      break;

    default:
      error("invalid state in update_game_state");
      break;
  }

  //  interrupts();

  return next_state;
}

void buttonPressInterrupt(){
  if(current_state == GAME_OVER){
    restart_flag = true;
  }
}
