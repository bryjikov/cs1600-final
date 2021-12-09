/**
 * testing.h
 * Exported testing functionality.
 */

typedef struct state_vars {
  bool game_over_flag;                     /* Set when the player has collided with an obstacle */
  bool pre_direction_change_flag;          /* Set when a direction change is upcoming and user should be warned */
  unsigned long time_entered_pdc;          /* Time (ms) at which the pre direction change state was entered */
  bool restart_flag;                       /* Set when the game should start over */
  unsigned obstacle_move_interval;         /* How often (ms) obstacles move */
  unsigned long time_last_obstacle_move;   /* Time (ms) of last obstacle movement */
  unsigned long time_last_dir_chg;         /* Time (ms) of last direction change event */
  unsigned long time_last_speed_up;        /* Time (ms) that obstacles were last sped up */
  unsigned long time_entered_setup;        /* Time (ms) when SETUP state was entered */
  direction_t obstacle_direction;          /* Direction of movement for obstacles */
  int player_x;                            /* Current X position of the player */
  int player_y;                            /* Current Y position of the player */
  bool moved;                              /* Set if either the player or obstacles moved since the last display */
  unsigned long start_time;                /* The time (in milis) when the player begins */
  unsigned long duration;                  /* The total time that the game lasted */
  LinkedPointerList<obstacle_t> *all_obstacles;     /* List containing all currently active obstacles */
  state_t current_state;                   /* state of the fsm */
} state_vars_t;

typedef struct test_case {
  state_vars_t input;
  state_vars_t output;
  unsigned long mils;

  /* These fields are used to expect what the mocked globals should
    be set to once the test case has finished running. */
  int mock_led_value;                         /* PWM value of LED when in PRE_DIRECTION_CHANGE */
  byte mock_displayed_player_x;               /* Player X as displayed by display_player() */
  byte mock_displayed_player_y;               /* Player Y as displayed by display_player() */
  unsigned mock_num_display_obstacle_calls;   /* Number of times display_obstacle() was called */
  bool mock_setup_was_displayed;              /* Was display_setup() called? */
  bool mock_game_over_was_displayed;          /* Was display_game_over() called? */
} test_case_t;


/*
 * Assert that a condition is true and print a success/failure 
 * message accordingly.
 * 
 * Note: The test_assert macro converts each condition to a string and 
 * indicates that these strings should be stored in flash memory. So, we
 * pass the condition_str as (__FlashStringHelper *), which Serial.println knows 
 * how to print (it can't be printed as a normal string since it's in flash)
 */
static void test_assert_underyling(bool condition, __FlashStringHelper *condition_str)
{
  if (!condition) {
    Serial.print(F("[FAILURE] "));
    Serial.println(condition_str);
  } else {
    PRINTLN_FLASH("[PASS]");
  }
}

/*
 * Assert a condition is true.
 * 
 * This wrapper converts the condition to a string
 * which can be printed if the assertion fails.
 */
#define test_assert(cond) test_assert_underyling(cond, F(#cond))

/**
 * Globals for mocked functions
 */
int mock_joy_x_initial;                     /* Allows control of initial joystick X reading */
int mock_joy_y_initial;                     /* Ditto for initial joystick Y reading */
int mock_joy_x;                             /* Allows control of joystick X reading */
int mock_joy_y;                             /* Ditto for joystick Y reading */
int mock_led_value;                         /* PWM value of LED when in PRE_DIRECTION_CHANGE */
byte mock_displayed_player_x;               /* Player X as displayed by display_player() */
byte mock_displayed_player_y;               /* Player Y as displayed by display_player() */
unsigned mock_num_display_obstacle_calls;   /* Number of times display_obstacle() was called */
bool mock_setup_was_displayed;              /* Was display_setup() called? */
bool mock_game_over_was_displayed;          /* Was display_game_over() called? */