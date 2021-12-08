#include <LinkedPointerList.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include "obstacles.h"
#include "display.h"
#include "led.h"
#include "logging.h"
#include "testing.h"

// Uncomment this for test mode
#define TESTING

#ifdef BIG_LCD
LiquidCrystal_I2C lcd(0x27, LCD_X_DIM, LCD_Y_DIM);
#else
const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#endif

/* Joystick Pins */
#define JOY_X A0
#define JOY_Y A1
#define JOY_BUTTON 2

/* FSM Constants and Variables */
#define PRE_DIR_CHG_DURATION              2000    /* How long to remain in PDC state (ms) */
#define DIR_CHG_INTERVAL                  10000   /* How often (ms) a direction change potentially happens */
#define SPEED_UP_INTERVAL                 3000    /* How often (ms) obstacles are sped up */
#define INIT_OBSTACLE_MOVE_INTERVAL       1000    /* How often obstacles are moved, initially */
#define OBSTACLE_MOVE_INTERVAL_DECREASE   50      /* By how much (ms) does the obstacle move interval decrease */
#define SETUP_WAIT_DURATION               2000    /* How long to wait in SETUP state (ms) */

volatile bool game_over_flag;                     /* Set when the player has collided with an obstacle */
volatile bool pre_direction_change_flag;          /* Set when a direction change is upcoming and user should be warned */
volatile unsigned long time_entered_pdc;          /* Time (ms) at which the pre direction change state was entered */
volatile bool restart_flag;                       /* Set when the game should start over */
volatile unsigned obstacle_move_interval;         /* How often (ms) obstacles move */
volatile unsigned long time_last_obstacle_move;   /* Time (ms) of last obstacle movement */
volatile unsigned long time_last_dir_chg;         /* Time (ms) of last direction change event */
volatile unsigned long time_last_speed_up;        /* Time (ms) that obstacles were last sped up */
volatile unsigned long time_entered_setup;        /* Time (ms) when SETUP state was entered */
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

void setup()
{
  Serial.begin(9600);

#ifdef TESTING
  // Wait for serial monitor to open, run tests, and do nothing else
  while (!Serial);
  run_all_tests();
  while (true);
#else

  srand(time(NULL));  // Set the random seed

  pinMode(LED_PIN, OUTPUT);
  pinMode(JOY_BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(JOY_BUTTON), joystick_button_press, RISING);

  all_obstacles = new LinkedPointerList<obstacle_t>();

  initialize_lcd();
  initialize_joystick();

  timer_setup();

  stop_watchdog();
  setup_watchdog();

  // Enter the SETUP state to kick off the FSM
  current_state = SETUP;
  time_entered_setup = millis();
  display_setup();

#endif
}

void loop()
{
  pet_watchdog();
  atomically_update_current_state(update_game_state(millis()));
}

/*
   Updates the current_state global, but ensures that no interrupt
   handler is concurrently accessing it.
*/
void atomically_update_current_state(state_t new_state)
{
  noInterrupts();
  current_state = new_state;
  interrupts();
}

/*
   Sets all FSM variables back to their initial states.
*/
void reset_fsm_variables(unsigned long mils) {
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
  time_entered_setup = 0;
  player_x = LCD_X_DIM / 2; // Start the player at the bottom and middle of the screen
  player_y = LCD_Y_MAX;
  obstacle_direction = LEFT;
  moved = true;
  free_all(all_obstacles);
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
    debug("Moving obstacles");

    // Move obstacles in the current direction and eliminate any that are out of bounds
    move_obstacles(all_obstacles, obstacle_direction);
    remove_out_of_bounds(all_obstacles);

    // Spawn a new obstacle with probability 1/4 (under TESTING,
    // always spawn a new obstacle)
#ifdef TESTING
    if (true) {
#else
    if (rand() % 4 == 0) {
#endif
      debug("Spawning new obstacle");
      spawn_random_obstacle(all_obstacles, obstacle_direction);
    }

    time_last_obstacle_move = mils;
    moved = true;
  }
  // If it has been long enough since last obstacle speed-up
  if (mils - time_last_speed_up > SPEED_UP_INTERVAL) {

    if (obstacle_move_interval >= OBSTACLE_MOVE_INTERVAL_DECREASE) {
      debug("Speeding up obstacles (move interval: decreasing from %d to %d)",
            obstacle_move_interval, obstacle_move_interval - OBSTACLE_MOVE_INTERVAL_DECREASE);
      obstacle_move_interval -= OBSTACLE_MOVE_INTERVAL_DECREASE;
    } else {
      debug("Obstacles at max speed (move interval: going from %d to 0)", obstacle_move_interval);
      obstacle_move_interval = 0;
    }

    time_last_speed_up = mils;
  }
  // If it has been long enough since last direction change event
  if (mils - time_last_dir_chg > DIR_CHG_INTERVAL) {
    debug("Possibly triggering direction change");

    // With probability 1/4, trigger a direction change (under
    // TESTING, always cause direction change).
#ifdef TESTING
    if (true) {
#else
    if (rand() % 4 == 0) {
#endif
      debug("Direction change");
      pre_direction_change_flag = true;
      time_entered_pdc = mils;
    }
    time_last_dir_chg = mils;
  }
  // If a collision is detected given the current position of the player
  // and configuration of the obstacles, indicate that it is game over.
  if (collision_detected(all_obstacles, player_x, player_y)) {
    debug("Collision detected (player at (%d, %d))", player_x, player_y);

    game_over_flag = true;
    duration = mils - start_time;
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
*/
state_t update_game_state(unsigned long mils)
{
  // By default, remain in the current state
  state_t next_state = current_state;

  switch (current_state) {
    case SETUP:
      // If we've waited in SETUP long enough
      if (mils - time_entered_setup >= SETUP_WAIT_DURATION) { // Transition 1-2
        debug("Transition: SETUP -> NORMAL");
        reset_fsm_variables(mils);
        next_state = NORMAL;
        start_time = mils;
      }
      break;

    case GAME_OVER:
      if (restart_flag) { // Transition 4-1
        debug("Transition: GAME_OVER -> SETUP");
        next_state = SETUP;
        time_entered_setup = mils;
        display_setup();
        restart_flag = false;
      }
      break;

    case NORMAL:
      // Collision has occurred; game over.
      if (game_over_flag) { // Transition 2-4
        debug("Transition: NORMAL -> GAME_OVER");
        next_state = GAME_OVER;
        game_over_flag = false;
        display_game_over(duration);
      }
      // It is time for a direction change
      else if (pre_direction_change_flag) { // Transition 2-3
        debug("Transition: NORMAL -> PRE_DIRECTION_CHANGE");
        next_state = PRE_DIRECTION_CHANGE;
        pre_direction_change_flag = false;
      } else {  // Transition 2-2
        update_for_normal_gameplay(mils);
      }
      break;

    case PRE_DIRECTION_CHANGE:
      // It's possible a collision occurs during pre-direction change
      if (game_over_flag) { // Transition 3-4
        debug("Transition: PRE_DIRECTION_CHANGE -> GAME_OVER");
        next_state = GAME_OVER;
        game_over_flag = false;
        display_game_over(duration);
      }
      // If it has been long enough since the pre direction change state was
      // entered, flip the direction of obstacle movement and transition to NORMAL.
      else if (mils - time_entered_pdc >= PRE_DIR_CHG_DURATION) { // Transition 3-2
        debug("Transition: PRE_DIRECTION_CHANGE -> NORMAL");
        obstacle_direction = invert_direction(obstacle_direction);
        next_state = NORMAL;
      } else {  // Transition 3-3
        update_for_normal_gameplay(mils);
      }
      break;

    default:
      halt_with_error("invalid current state: %d", current_state);
      break;
  }

  return next_state;
}
