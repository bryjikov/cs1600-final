/*
   watchdog.ino
   Watchdog timer.

   Note: These functions only do anything if TESTING mode is disabled.
   Under testing, they are not needed (and will break if run on MKR).
*/
#ifndef TESTING
#include <avr/wdt.h>
#endif

/*
   disables the watchdog
*/
void stop_watchdog(void)
{
#ifndef TESTING
  debug("Disabling watchdog timer");
  wdt_disable();
  delay(3000); //idk the internet recommends this
#endif
}

/*
   Create a watchdog timer that is set to reset the system
   after 4 seconds without being reset.
*/
void setup_watchdog(void)
{
#ifndef TESTING
  debug("Enabling watchdog timer");
  wdt_enable(WDTO_4S); //4 second timeout. idk if we really need this
#endif
}

/*
   Pets the watchdog so that it does not reset the system.
*/
void pet_watchdog(void)
{
#ifndef TESTING
  wdt_reset();
#endif
}
