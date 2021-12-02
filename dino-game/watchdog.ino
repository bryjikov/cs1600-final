/*
   watchdog.ino
   Watchdog timer.
*/
#include <avr/wdt.h>

/*
   disables the watchdog
*/
void stop_watchdog(void)
{
  debug("Disabling watchdog timer");
  wdt_disable();
  delay(3000); //idk the internet recommends this
}

/*
   Create a watchdog timer that is set to reset the system
   after 4 seconds without being reset.
*/
void setup_watchdog(void)
{
  debug("Enabling watchdog timer");
  wdt_enable(WDTO_4S); //4 second timeout. idk if we really need this
}

/*
   Pets the watchdog so that it does not reset the system.
*/
void pet_watchdog(void)
{
  wdt_reset();
}
