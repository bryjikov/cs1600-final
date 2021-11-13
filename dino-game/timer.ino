/*
   timer.ino
   Timer-related functionality.
*/

unsigned long driver_counter = 0;

/*
   Calls the handler for any jobs whose interval multiples divide
   the current driver counter.
*/
void invoke_driver(void)
{
  job_t job;
  for (int i = 0; i < all_jobs->size(); i++) {
    job = all_jobs->get(i);

    if (driver_counter % job.interval_multiple == 0) {
      Serial.print("Invoking job ");
      Serial.print(job.id);
      Serial.print("at millis: ");
      Serial.println(millis());

      job.handler();
    }
  }

  // FIXME: I'm pretty sure this approach won't work once driver_counter overflows.
  // However, millis() (an unsigned long) overflows after 50 days, so we could just make
  // the assumption that this *never* overflows while you're playing the game.
  //
  // If we knew all the interval multiples statically, we could manually reset this to 0
  // just before it hits the LCM of all the interval multiples. However, because the obstacle
  // move interval changes as the game progresses, this is impossible.
  driver_counter++;
}

/*
   Adds a job to the global list of jobs, given how often it should run
   (every interval milliseconds)
*/
void register_job(job_id_t id, void (*handler)(void), size_t interval)
{
  if (interval % DRIVER_INTERVAL != 0) {
    error("register_job: invalid interval for job");
  }

  size_t interval_multiple = interval / DRIVER_INTERVAL;

  job_t job = {
    id,
    handler,
    interval_multiple
  };
  all_jobs->add(job);
}

/*
   Retrive a job from the global jobs list by its id.
*/
job_t get_job(job_id_t id)
{
  job_t job;
  for (int i = 0; i < all_jobs->size(); i++) {
    job = all_jobs->get(i);

    if (job.id == id) {
      return job;
    }
  }

  error("tried to get invalid job");
}

/*
   Updates the interval multiple of a job in the global all_jobs list.
   (useful for changing the obstacle move interval)
*/
void update_interval_multiple(job_id_t id, size_t new_interval_multiple)
{
  job_t job;
  for (int i = 0; i < all_jobs->size(); i++) {
    job = all_jobs->get(i);

    if (job.id == id) {
      job.interval_multiple = new_interval_multiple;
      all_jobs->set(i, job);
      return;
    }
  }

  error("tried to update interval multiple of invalid job");
}

/*
   Checks if TC5 is done syncing.
*/
bool tcIsSyncing()
{
  return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}
/*
   This function enables TC5 and waits for it to be ready
*/
void tcStartCounter()
{
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; // set the CTRLA register
  while (tcIsSyncing()); // wait until snyc'd
}

void tcReset()
{
  TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (tcIsSyncing());
  while (TC5->COUNT16.CTRLA.bit.SWRST);
}

void tcDisable()
{
  TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
  while (tcIsSyncing());
}

/*
   Configures the TC to generate output events at the sample frequency.
*/
void tcConfigure(int sampleRate)
{
  // select the generic clock generator used as source to the generic clock multiplexer
  GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID(GCM_TC4_TC5));
  while (GCLK->STATUS.bit.SYNCBUSY);

  tcReset(); // reset TC5

  // Set Timer counter 5 Mode to 16 bits, it will become a 16bit counter ('mode1' in the datasheet)
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
  // Set TC5 waveform generation mode to 'match frequency'
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
  // set prescaler
  TC5->COUNT16.CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_ENABLE; // it will divide GCLK_TC frequency by 1024
  // set the compare-capture register.
  TC5->COUNT16.CC[0].reg = (uint16_t)(SystemCoreClock / sampleRate);
  while (tcIsSyncing());

  // Configure interrupt request
  NVIC_DisableIRQ(TC5_IRQn);
  NVIC_ClearPendingIRQ(TC5_IRQn);
  NVIC_SetPriority(TC5_IRQn, 0);
  NVIC_EnableIRQ(TC5_IRQn);

  // Enable the TC5 interrupt request
  TC5->COUNT16.INTENSET.bit.MC0 = 1;
  while (tcIsSyncing()); // wait until TC5 is done syncing
}

/*
   Handler for the TC5 timer, which we are using to invoke the driver
   at a regular interval. The driver then invokes other timed jobs as necessary.
*/
void TC5_Handler(void)
{
  tcDisable();

  Serial.print("Invoking timer driver at millis ");
  Serial.println(millis());
  invoke_driver();

  TC5->COUNT16.INTFLAG.bit.MC0 = 1; // Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}
