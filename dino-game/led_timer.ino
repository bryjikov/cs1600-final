
#define PWM_MIN 0
#define PWM_MAX 255

int ledBrightness = 0;
bool brightnessAscending = true;

void TC4_Handler(void)
{
  tc4Disable();

  Serial.println("In TC4 handler!!");

  // set the LED at current brightness
  analogWrite(ledPin, ledBrightness);

  // change direction of pulsing if at lower/upper ends
  if (ledBrightness == PWM_MIN || ledBrightness == PWM_MAX) {
    brightnessAscending = !brightnessAscending;
  }

  // increase/decrease brightness
  if (brightnessAscending) {
    ledBrightness++;
  } else {
    ledBrightness--;
  }

  TC4->COUNT16.INTFLAG.bit.MC0 = 1; // Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}

/*
    Adapted from: https://gist.github.com/nonsintetic/ad13e70f164801325f5f552f84306d6f

    Sets up TC4 to be timer for
*/
void tc4Configure(int sampleRate)
{
  // select the generic clock generator used as source to the generic clock multiplexer
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;

  Serial.println(GCM_TC4_TC5, HEX);

  tc4Reset(); // reset TC4

  // Set Timer counter 4 Mode to 16 bits, it will become a 16bit counter ('mode1' in the datasheet)
  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  // Set TC4 waveform generation mode to 'match frequency'
  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  // set prescaler
  // the clock normally counts at the GCLK_TC frequency, but we can set it to divide that frequency to slow it down
  // you can use different prescaler divisons here like TC_CTRLA_PRESCALER_DIV1 to get a different range
  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  // set the compare-capture register.
  // The counter will count up to this value (it's a 16bit counter so we use uint16_t)
  // this is how we fine-tune the frequency, make it count to a lower or higher value
  // system clock should be 1MHz (8MHz/8) at Reset by default
  TC4->COUNT16.CC[0].reg = (uint16_t)(SystemCoreClock / sampleRate);
  while (tc4IsSyncing())
    ;

  // Configure interrupt request
  NVIC_DisableIRQ(TC4_IRQn);
  NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);
  NVIC_EnableIRQ(TC4_IRQn);

  // Enable the TC4 interrupt request
  TC4->COUNT16.INTENSET.bit.MC0 = 1;
  while (tc4IsSyncing())
    ; // wait until TC4 is done syncing
}

// Function that is used to check if TC4 is done syncing
// returns true when it is done syncing
bool tc4IsSyncing(void)
{
  return TC4->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

// This function enables TC4 and waits for it to be ready
void tc4StartCounter(void)
{
  TC4->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; // set the CTRLA register
  while (tc4IsSyncing())
    ; // wait until snyc'd
}

// Reset TC4
void tc4Reset(void)
{
  TC4->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tc4IsSyncing())
    ;
  while (TC4->COUNT16.CTRLA.bit.SWRST)
    ;
}

// disable TC4
void tc4Disable(void)
{
  TC4->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tc4IsSyncing())
    ;
}
