/*
 * joystick.ino
 * Functionality pertaining to the joystick used to control player
 * movement and game resets.
 */

/*
 * Analog pins used for reading the X and Y position of the joystick,
 * and the digital pin used for handling joystick button presses.
 */

int joystickPrevPosX = 0;
int joystickPrevPosY = 0;
int joystickInitialPosX;
int joystickInitialPosY;

/*
 * Initialize the joystick by taking an initial reading of the 
 * joystick position to compare with later.
 */
void initialize_joystick(void)
{
#ifdef TESTING
  // TODO: under testing, these should be set by globals
  joystickInitialPosX = 0;
  joystickInitialPosY = 0;
#else
  joystickInitialPosX = analogRead(JOY_X);
  joystickInitialPosY = analogRead(JOY_Y);
#endif
}

/*
 * How much change from the initial joystick reading should constitute a move.
 */
#define JOYSTICK_CHANGE_THRESHOLD 15

/*
 * Converts from the joystick's analog value to one of {-1, 0, 1} 
 * depending on whether it is within a threshold of its initial value.
 */
int convert_joystick_pos(int pos, int init)
{
    if (pos >= init - JOYSTICK_CHANGE_THRESHOLD && pos <= init + JOYSTICK_CHANGE_THRESHOLD) {
        return 0;
    } else if (pos > init + JOYSTICK_CHANGE_THRESHOLD) {
        return 1;
    } else {
        return -1;
    }
}

/*
  Read the current joystick position, and if movement is detected, update
  the player's position accordingly.
*/
void update_joystick(void)
{
#ifdef TESTING
  // TODO: Set these using globals controlled by tester
  int xValue = 0;
  int yValue = 0;
#else
  int xValue = analogRead(JOY_X);
  int yValue = analogRead(JOY_Y);
#endif

  int joystickPosX = convert_joystick_pos(xValue, joystickInitialPosX);
  int joystickPosY = convert_joystick_pos(yValue, joystickInitialPosY);

  if (joystickPosX != joystickPrevPosX || joystickPosY != joystickPrevPosY) {
    // Constrain player position to be within bounds of display
    player_x = constrain(player_x + joystickPosX, LCD_X_MIN, LCD_X_MAX);
    player_y = constrain(player_y + joystickPosY, LCD_Y_MIN, LCD_Y_MAX);
    moved = true;
  }

  joystickPrevPosX = joystickPosX;
  joystickPrevPosY = joystickPosY;
}

/*
   Interrupt handler for button presses on the joystick. If currently
   in a game over state, the game is restarted. Otherwise, presses are ignored.
*/
void joystick_button_press(void)
{
  if (current_state == GAME_OVER) {
    debug("Joystick button pressed in GAME_OVER: resetting");
    restart_flag = true;
  }
}


#ifdef TESTING

/**
 * Unit tests for joystick functions
 */
void test_joystick(void)
{
  PRINTLN_FLASH("Testing convert_joystick_pos...");
  test_assert(convert_joystick_pos(10, 10) == 0);
  test_assert(convert_joystick_pos(400, 400 - JOYSTICK_CHANGE_THRESHOLD) == 0);
  test_assert(convert_joystick_pos(70, 70 + (2 * JOYSTICK_CHANGE_THRESHOLD)) == -1);
  test_assert(convert_joystick_pos(350, 350 - (4 * JOYSTICK_CHANGE_THRESHOLD)) == 1);
  PRINTLN_FLASH("Testing joystick_button_press...");
  // No change during SETUP
  current_state = SETUP; restart_flag = false;
  joystick_button_press();
  test_assert(!restart_flag);
  // No change during NORMAL
  current_state = NORMAL; restart_flag = false;
  joystick_button_press();
  test_assert(!restart_flag);
  // No change during PRE_DIRECTION_CHANGE
  current_state = PRE_DIRECTION_CHANGE; restart_flag = false;
  joystick_button_press();
  test_assert(!restart_flag);
  // Flag set when in GAME_OVER
  current_state = GAME_OVER; restart_flag = false;
  joystick_button_press();
  test_assert(restart_flag);
}

#endif
