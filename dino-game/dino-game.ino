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

const int rs = 0, en = 1, d4 = 2, d5 = 3, d6 = 4, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int buttonPin = 8;

int player_y = 1;

/* FSM Variables */
volatile bool game_over_flag = false;             /* Set when the player has collided with an obstacle */
volatile bool pre_direction_change_flag = false;  /* Set when a direction change is upcoming and user should be warned */
volatile unsigned long time_entered_pdc;          /* Time (millis()) at which the pre direction change state was entered */
volatile bool restart_flag = false;               /* Set when the game should start over */
volatile direction_t obstacle_direction = LEFT;   /* Direction of movement for obstacles */

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
  initialize_fsm();

  pinMode(buttonPin, INPUT);
  pinMode(LED_PIN, OUTPUT);

  // Configure the timer driver to run every DRIVER_INTERVAL ms
  tcConfigure(DRIVER_INTERVAL);

  all_obstacles = new LinkedPointerList<obstacle_t>();
  all_jobs = new LinkedPointerList<job_t>();

  register_job(MOVE_OBSTACLES, &obstacle_move_handler, 250);
  register_job(SPEED_UP_OBSTACLES, &obstacle_speed_up_handler, 1000);
}

void loop()
{
  //pet_watchdog();
  current_state = update_game_state(millis());
  display_player(8, 1);
  //display_obstacles(all_obstacles);
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
  player_y = 1;
  obstacle_direction = LEFT;
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
      if (game_over_flag) {
        next_state = GAME_OVER;
        game_over_flag = false;
      } else if (pre_direction_change_flag) {
        next_state = PRE_DIRECTION_CHANGE;
        pre_direction_change_flag = false;
      }
      break;

    case PRE_DIRECTION_CHANGE:
      // If it has been long enough since the pre direction change state was
      // entered, flip the direction of obstacle movement and transition to NORMAL.
      if (mils - time_entered_pdc >= 2000) {
        obstacle_direction = invert_direction(obstacle_direction);
        next_state = NORMAL;
      }
      break;

    default:
      error("invalid state in update_game_state");
      break;
  }

  return next_state;
}
