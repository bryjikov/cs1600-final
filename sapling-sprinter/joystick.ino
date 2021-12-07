/**
 * joystick.ino
 * Functionality pertaining to the joystick used to control player
 * movement and game resets.
 */

/**
 * Analog pins used for reading the X and Y position of the joystick,
 * and the digital pin used for handling joystick button presses.
 */
#define JOY_X A0
#define JOY_Y A1
#define JOY_BUTTON 2

int joystickPrevPosX = 0;
int joystickPrevPosY = 0;
int joystickInitialPosX;
int joystickInitialPosY;

/**
 * Initialize the joystick by taking an initial reading of the 
 * joystick position to compare with later.
 */
void initialize_joystick(void)
{
  // TODO: under testing, this needs to be different.
  joystickInitialPosX = analogRead(JOY_X);
  joystickInitialPosY = analogRead(JOY_Y);
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
  Update Joystick position and map the joysticks value (0, 1023) to
  (-1, 1) where it is only 0 if at center position (500, 515).
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
