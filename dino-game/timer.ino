/* JUMP HANDLER */
void TC5_Handler(void)
{
    tcDisable();
    jump_end_flag = true;
    TC5->COUNT16.INTFLAG.bit.MC0 = 1; // Writing a 1 to INTFLAG.bit.MC0 clears the interrupt so that it will run again
}

// Configures the TC to generate output events at the sample frequency.
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

// Function that is used to check if TC5 is done syncing
bool tcIsSyncing()
{
    return TC5->COUNT16.STATUS.reg & TC_STATUS_SYNCBUSY;
}

// This function enables TC5 and waits for it to be ready
void tcStartCounter()
{
    TC5->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE; // set the CTRLA register
    while (tcIsSyncing()); // wait until snyc'd
}

// Reset TC5
void tcReset()
{
    TC5->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
    while (tcIsSyncing());
    while (TC5->COUNT16.CTRLA.bit.SWRST);
}

// disable TC5
void tcDisable()
{
    TC5->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
    while (tcIsSyncing());
}