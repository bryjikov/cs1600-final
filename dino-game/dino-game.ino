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
#define BUTTON_PIN 8
#define PRE_DIR_CHG_DURATION 2000 /* How long to remain in PDC state (ms) */

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

/* FSM Variables */
volatile bool game_over_flag;             /* Set when the player has collided with an obstacle */
volatile bool pre_direction_change_flag;  /* Set when a direction change is upcoming and user should be warned */
volatile unsigned long time_entered_pdc;  /* Time (millis()) at which the pre direction change state was entered */
volatile bool restart_flag;               /* Set when the game should start over */
volatile direction_t obstacle_direction;  /* Direction of movement for obstacles */
volatile int player_x;                    /* Current X position of the player */
volatile int player_y;                    /* Current Y position of the player */

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

void obstacle_move_handler(void)
{
  // Move obstacles in the current direction and eliminate any that are out of bounds
  move_obstacles(all_obstacles, obstacle_direction);
  remove_out_of_bounds(all_obstacles);

  // Spawn a new obstacle with probability 1/8
  if (rand() % 8 == 0) {
    spawn_random_obstacle(all_obstacles, obstacle_direction);
  }
}

void obstacle_speed_up_handler(void)
{
  job_t *job = get_job(MOVE_OBSTACLES);
  // Subtracting 1 is 50ms less because this is in terms of the DRIVER_INTERVAL (which is 50ms)
  update_interval_multiple(MOVE_OBSTACLES, job->interval_multiple - 1);
}

void direction_change_handler(void)
{
  // With probability 1/4, trigger a direction change
  if (rand() % 4 == 0) {
    pre_direction_change_flag = true;
    time_entered_pdc = millis();
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  initialize_lcd();
  initialize_fsm();

  // Configure the timer driver to run every DRIVER_INTERVAL ms
  tcConfigure(DRIVER_INTERVAL);

  all_obstacles = new LinkedPointerList<obstacle_t>();
  all_jobs =      new LinkedPointerList<job_t>();
}

void loop()
{
  pet_watchdog();
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
  player_x = 8;
  player_y = 1;
  obstacle_direction = LEFT;

  clear_jobs();
  register_job(MOVE_OBSTACLES, &obstacle_move_handler, 250);
  register_job(SPEED_UP_OBSTACLES, &obstacle_speed_up_handler, 1000);
  register_job(DIRECTION_CHANGE, &direction_change_handler, 10000);
}

/*
   Updates the FSM, given the current time in milliseconds.

   TODO: label all transitions according to an FSM diagram, which we
   also need to update.
*/
state_t update_game_state(long mils)
{
  // By default, remain in the current state
  state_t next_state = current_state;

  // FIXME: does masking off interrupts prevent the timer handler from getting
  // invoked? That's what I'm trying to prevent here. 
  noInterrupts(); // mask interrupts to protect access to globals

  // If a collision is detected given the current position of the player
  // and configuration of the obstacles, indicate that it is game over.
  if (collision_detected(all_obstacles, player_x, player_y)) {
    game_over_flag = true;
  }

  switch (current_state) {
    case SETUP:
      initialize_fsm();
      next_state = NORMAL;
      break;

    case GAME_OVER:
      if (restart_flag) {
        next_state = SETUP;
        restart_flag = false;
      }
      break;

    case NORMAL:
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
