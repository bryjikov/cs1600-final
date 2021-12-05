/*
   led.ino
   Controls LED pulsating before a direction change.
*/

// Range of PWM values for analogWrite
#define PWM_MIN 255
#define PWM_MAX 0

int led_brightness = 0;
bool brightness_ascending = true;

/**
   Update the pulsing LED's brightness to cycle between the
   min and max PWM values, so the LED pulsates constantly.
*/
void updateLED(void)
{
  // Only pulsate the LED in the pre-direction change state
  if (current_state == PRE_DIRECTION_CHANGE) {
    // set the LED at current brightness
    analogWrite(LED_PIN, led_brightness);

    // increase/decrease brightness
    if (brightness_ascending) {
      led_brightness++;
    } else {
      led_brightness--;
    }

    // change direction of pulsing if at lower/upper ends
    if (led_brightness == PWM_MIN || led_brightness == PWM_MAX) {
      brightness_ascending = !brightness_ascending;
    }
  } else {
    analogWrite(LED_PIN, PWM_MIN);
  }
}
