/**
 * testing.h
 * Exported testing functionality.
 */

/*
 * Assert that a condition is true and print a success/failure 
 * message accordingly.
 */
static void test_assert_underyling(bool condition, char *condition_str)
{
  if (!condition) {
    serial_printf("[FAILURE] %s\n", condition_str);
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
#define test_assert(cond) test_assert_underyling(cond, #cond)    // TODO: can these strings be in flash