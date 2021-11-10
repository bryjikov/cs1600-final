/*
   watchdog.ino
   Watchdog timer.
*/

/*
   Create a watchdog timer that is set to reset the system
   after 4 seconds without being reset.
*/
void setup_watchdog()
{
  // Clear and enable WDT
  NVIC_DisableIRQ(WDT_IRQn);
  NVIC_ClearPendingIRQ(WDT_IRQn);
  NVIC_SetPriority(WDT_IRQn, 0);
  NVIC_EnableIRQ(WDT_IRQn);

  // Configure and enable WDT GCLK:
  // Configure and enable WDT GCLK:
  GCLK->GENDIV.reg = GCLK_GENDIV_DIV(4) | GCLK_GENDIV_ID(5);
  while (GCLK->STATUS.bit.SYNCBUSY);

  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(5) | GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC(3) | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY);
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(3) | GCLK_CLKCTRL_GEN(5) | GCLK_CLKCTRL_CLKEN;
  while (GCLK->STATUS.bit.SYNCBUSY);

  // Configure and enable WDT:
  WDT->CONFIG.reg = WDT_CONFIG_PER(9);
  WDT->EWCTRL.reg = WDT_EWCTRL_EWOFFSET(8);
  WDT->CTRL.reg = WDT_CTRL_ENABLE;

  // Enable early warning interrupts on WDT:
  // reference WDT registers with WDT->register_name.reg
  WDT->INTENSET.reg = WDT_INTENSET_EW;
  while (WDT->STATUS.bit.SYNCBUSY);
}

/*
   Pets the watchdog so that it does not reset the system.
*/
void pet_watchdog(void)
{
  WDT->CLEAR.reg = WDT_CLEAR_CLEAR(0xa5);
}

/*
   Handle an early-warning interrupt from the watchdog timer.

   TODO: do we actually need to use this?
*/
void WDT_Handler()
{
  // Clear interrupt register flag
  // (reference register with WDT->register_name.reg)
  WDT->INTFLAG.reg = WDT_INTFLAG_RESETVALUE;

  // Warn user that a watchdog reset may happen
  //Serial.println("watch dog timer early warning");
}
