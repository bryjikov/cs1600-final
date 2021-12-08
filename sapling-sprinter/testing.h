/**
 * testing.h
 * Exported testing functionality.
 */

/*
 * Assert that a condition is true and print a success/failure 
 * message accordingly.
 * 
 * Note: The test_assert macro converts each condition to a string and 
 * indicates that these strings should be stored in flash memory. So, we
 * pass the condition_str as (__FlashStringHelper *), which Serial.println knows 
 * how to print (it can't be printed as a normal string since it's in flash)
 */
static void test_assert_underyling(bool condition, __FlashStringHelper *condition_str)
{
  if (!condition) {
    Serial.print(F("[FAILURE] "));
    Serial.println(condition_str);
  } else {
    PRINTLN_FLASH("[PASS]");
  }
}

/*
 * Assert a condition is true.
 * 
 * This wrapper converts the condition to a string
 * which can be printed if the assertion fails.
 */
#define test_assert(cond) test_assert_underyling(cond, F(#cond))