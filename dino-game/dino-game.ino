#include <LiquidCrystal.h>
#include <LinkedPointerList.h>
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

#define joyX A0
#define joyY A1
#define BUTTON_PIN 8

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* FSM Constants and Variables */
#define PRE_DIR_CHG_DURATION              2000    /* How long to remain in PDC state (ms) */
#define DIR_CHG_INTERVAL                  10000   /* How often (ms) a direction change potentially happens */
#define SPEED_UP_INTERVAL                 1000    /* How often (ms) obstacles are sped up */
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

state_t current_state = SETUP;

// Prints an error message and halts the system
void error(String msg) {
  Serial.println(msg);
  while (true);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  initialize_lcd();
  initialize_fsm();

  // TODO: figure out how often we want this timer going off
  tcConfigure(1000);

  all_obstacles = new LinkedPointerList<obstacle_t>();

  // setup_watchdog();
}

void loop()
{
  // pet_watchdog();
  current_state = update_game_state(millis());
  display_player(player_x, player_y);
  display_obstacles(all_obstacles);
  updateLED();
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
  player_x = 8;
  player_y = 1;
  obstacle_direction = LEFT;
  all_obstacles->clear();
}

/*
   Checks any conditions and updates any state variables for normal gameplay
   (states NORMAL and PRE_DIRECTION_CHANGE). This involves detecting collisions,
   moving obstacles, speeding up obstacles, and causing direction changes.
*/
void update_for_normal_gameplay(unsigned long mils)
{
  // If a collision is detected given the current position of the player
  // and configuration of the obstacles, indicate that it is game over.
  if (collision_detected(all_obstacles, player_x, player_y)) {
    game_over_flag = true;
    return; // Bail out here, since the game is over
  }
  // If it has been long enough since last obstacle move
  if (mils - time_last_obstacle_move > obstacle_move_interval) {
    // Move obstacles in the current direction and eliminate any that are out of bounds
    move_obstacles(all_obstacles, obstacle_direction);
    remove_out_of_bounds(all_obstacles);

    // Spawn a new obstacle with probability 1/8
    if (rand() % 8 == 0) {
      spawn_random_obstacle(all_obstacles, obstacle_direction);
    }

    time_last_obstacle_move = mils;
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

  noInterrupts(); // Mask interrupts to protect access to globals

  switch (current_state) {
    case SETUP:
      initialize_fsm();
      next_state = NORMAL;
      break;

    case GAME_OVER:
      display_game_over();
      if (restart_flag) {
        next_state = SETUP;
        restart_flag = false;
      }
      break;

    case NORMAL:
      update_for_normal_gameplay(mils);

      // Collision has occurred; game over.
      if (game_over_flag) {
        next_state = GAME_OVER;
        game_over_flag = false;
        // It is time for a direction change
      } else if (pre_direction_change_flag) {
        next_state = PRE_DIRECTION_CHANGE;
        pre_direction_change_flag = false;
      }
      break;

    case PRE_DIRECTION_CHANGE:
      update_for_normal_gameplay(mils);

      // It's possible a collision occurs during pre-direction change
      if (game_over_flag) {
        next_state = GAME_OVER;
        game_over_flag = false;
      }
      // If it has been long enough since the pre direction change state was
      // entered, flip the direction of obstacle movement and transition to NORMAL.
      else if (mils - time_entered_pdc >= PRE_DIR_CHG_DURATION) {
        obstacle_direction = invert_direction(obstacle_direction);
        next_state = NORMAL;
      }
      break;

    default:
      error("invalid state in update_game_state");
      break;
  }

  interrupts();

  return next_state;
}
