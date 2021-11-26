/*
   timer.ino
   Timer-related functionality.
*/

void timer_setup(){
  //temporarily disable interrupts 
  noInterrupts();

  //initialize registers and counters to 0
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;

  //set count at for interrupt to occur, translates to 1kH frequency w/128 prescaler 
  //formula: (16 * 10^6) / (desired frequency * prescaler) - 1
  OCR2A = 124

  //turn on CTC mode
  TCCR2A |= (1 << WGM21);

  //set prescaler
  TCCR2B |= (1 << CS22) | 1 << CS20);  

  //enable timer interupt 
  TIMSK2 |= (1 << OCIE2A);

  //re-enable interrupts 
  interrupts();
}


ISR(TIMER2_COMPA_vect){
  //timer interrupt just calls updateLED
  updateLED();
}
