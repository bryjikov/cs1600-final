/**
 * testing.h
 * Exported testing functionality.
 */

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
int mock_led_value;                       /* PWM value of LED when in PRE_DIRECTION_CHANGE */
int mock_joy_x_initial;                     /* Allows control of initial joystick X reading */
int mock_joy_y_initial;                     /* Ditto for initial joystick Y reading */
int mock_joy_x;                             /* Allows control of joystick X reading */
int mock_joy_y;                             /* Ditto for joystick Y reading */
byte mock_displayed_player_x;               /* Player X as displayed by display_player() */
byte mock_displayed_player_y;               /* Player Y as displayed by display_player() */
unsigned mock_num_display_obstacle_calls;   /* Number of times display_obstacle() was called */
bool mock_setup_was_displayed;              /* Was display_setup() called? */
bool mock_game_over_was_displayed;          /* Was display_game_over() called? */