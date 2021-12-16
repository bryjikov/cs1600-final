/*
   timer.ino
   Timer-related functionality.

   Note: The timer functions only do something if TESTING is disabled.
   These aren't needed during testing mode and will break the MKR.
*/

void timer_setup() {
#ifndef TESTING
  debug("Setting up timer for LED");
  //temporarily disable interrupts
  noInterrupts();

  //initialize registers and counters to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  //set count at for interrupt to occur, translates to 1kH frequency w/128 prescaler
  //formula to determine OCR value: (16 * 10^6) / (desired frequency * prescaler) - 1 [src 1]
  OCR2A = 124;

  //turn on CTC mode (timer goes off when TCNT matches OCR)
  TCCR2A |= (1 << WGM21);

  //set prescaler
  TCCR2B |= (1 << CS22) | (1 << CS20);

  //enable timer interupt
  TIMSK2 |= (1 << OCIE2A);

  //re-enable interrupts
  interrupts();
#endif
}

#ifndef TESTING
ISR(TIMER2_COMPA_vect) {
  //timer interrupt just calls updateLED
  updateLED();
}
#endif

/*
   sources:
   1: https://maker.pro/arduino/projects/timer-interrupts-improve-your-arduino-programming-skills
*/
