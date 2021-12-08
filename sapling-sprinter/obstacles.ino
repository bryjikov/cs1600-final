/*
   obstacles.ino
   Functions for managing / updating obstacles.
*/

/*
   Produces the opposite of the given direction.
*/
direction_t invert_direction(direction_t dir)
{
  switch (dir) {
    case LEFT:
      return RIGHT;
    case RIGHT:
      return LEFT;
    default:
      halt_with_error("invalid direction: %d", dir);
  }
}

/*
   Moves each obstacle in a list 1 unit in the indicated direction.
*/
void move_obstacles(LinkedPointerList<obstacle_t> *obstacles, direction_t dir)
{
  obstacle_t *obs;
  for (int i = 0; i < obstacles->size(); i++) {
    obs = obstacles->get(i);

    switch (dir) {
      case LEFT:
        obs->x -= 1;
        break;
      case RIGHT:
        obs->x += 1;
        break;
      default:
        halt_with_error("invalid direction: %d", dir);
    }
  }
}

/*
   Removes any obstacles from the given list that are outside
   the bounds of the screen.
*/
void remove_out_of_bounds(LinkedPointerList<obstacle_t> *obstacles)
{
  obstacle_t *obs;
  LinkedPointerList<obstacle_t> only_in_bounds;

  for (int i = 0; i < obstacles->size(); i++) {
    obs = obstacles->get(i);

    // Only keep obstacles within bounds
    if (obs->x >= LCD_X_MIN && obs->x <= LCD_X_MAX && obs->y >= LCD_Y_MIN && obs->y <= LCD_Y_MAX)
    {
      only_in_bounds.add(obs);
    } else {
      // Obstacle is out of bounds, make sure to free its memory
      free(obs);
    }
  }

  // Replace original list with filtered list
  obstacles->clear();
  for (int i = 0; i < only_in_bounds.size(); i++) {
    obstacles->add(only_in_bounds.get(i));
  }
}

/*
   Generates a new obstacle at a random Y position and
   adds it to the given list of obstacles. The current
   direction is indicated so that obstacles can be spawned.
*/
void spawn_random_obstacle(LinkedPointerList<obstacle_t> *obstacles, direction_t dir)
{
  uint8_t x, y;

#ifdef TESTING
  y = 0;  // Always spawn at y=0 under TESTING mode
#else
  y = rand() % (LCD_Y_MAX + 1);
#endif

  // If obstacles are moving left, spawn on the right (LCD_X_MAX)
  // If obstacles are moving right, spawn on the left (LCD_X_MIN)
  switch (dir) {
    case LEFT:
      x = LCD_X_MAX;
      break;
    case RIGHT:
      x = LCD_X_MIN;
      break;
    default:
      halt_with_error("invalid direction: %d", dir);
  }

  // Add the new obstacle
  create_obstacle_at(x, y, obstacles);
}

/*
   Determines if the player at the given coordinates is colliding with
   any of the obstacles in the list.
*/
bool collision_detected(LinkedPointerList<obstacle_t> *obstacles, uint8_t player_x, uint8_t player_y)
{
  obstacle_t* obs;
  for (int i = 0; i < obstacles->size(); i++) {
    obs = obstacles->get(i);

    if (obs->x == player_x && obs->y == player_y) {
      return true;
    }
  }

  return false;
}

/*
   Displays all objects in the given list to the LCD.
*/
void display_obstacles(LinkedPointerList<obstacle_t> *obstacles)
{
  for (int i = 0; i < obstacles->size(); i++) {
    display_obstacle(obstacles->get(i));
  }
}

/*
   Frees every obstacle in the given list and then clears the list.
*/
void free_all(LinkedPointerList<obstacle_t> *obstacles)
{
  obstacle_t *o;
  for (int i = 0; i < obstacles->size(); i++) {
    o = obstacles->get(i);
    free(o);
  }
  obstacles->clear();
}

/*
   Allocates a new obstacle and adds it onto the given obstacle list. Returns
   a pointer to the created obstacle.
*/
obstacle_t *create_obstacle_at(uint8_t x, uint8_t y, LinkedPointerList<obstacle_t> *obstacles)
{
  obstacle_t *obs = (obstacle_t *)malloc(sizeof(obstacle_t));
  if (obs == NULL) {
    halt_with_error("malloc failed");
  }
  obs->x = x;
  obs->y = y;
  obstacles->add(obs);
  return obs;
}


#ifdef TESTING

/*
 * Unit tests for obstacle functions
 */
void test_obstacle_functions(void)
{
  // Obstacle list used for purposes of testing
  LinkedPointerList<obstacle_t> *obstacles = new LinkedPointerList<obstacle_t>();
  obstacle_t *a, *b, *c;

  PRINTLN_FLASH("Testing invert_direction...");
  test_assert(invert_direction(LEFT) == RIGHT);
  test_assert(invert_direction(RIGHT) == LEFT);

  PRINTLN_FLASH("Testing move_obstacles...");
  a = create_obstacle_at(3, 4, obstacles);
  b = create_obstacle_at(8, 0, obstacles);
  c = create_obstacle_at(13, 2, obstacles);
  move_obstacles(obstacles, LEFT);
  test_assert(a->x == 2 && a->y == 4);
  test_assert(b->x == 7 && b->y == 0);
  test_assert(c->x == 12 && c->y == 2);
  move_obstacles(obstacles, RIGHT);
  move_obstacles(obstacles, RIGHT);
  test_assert(a->x == 4 && a->y == 4);
  test_assert(b->x == 9 && b->y == 0);
  test_assert(c->x == 14 && c->y == 2);
  free_all(obstacles);

  PRINTLN_FLASH("Testing remove_out_of_bounds...");
  a = create_obstacle_at(LCD_X_MIN - 1, 2, obstacles);
  b = create_obstacle_at(7, LCD_Y_MAX + 1, obstacles);
  c = create_obstacle_at(0, 0, obstacles);
  remove_out_of_bounds(obstacles);
  test_assert(obstacles->size() == 1);
  test_assert(obstacles->get(0) == c);
  free_all(obstacles);

  PRINTLN_FLASH("Testing spawn_random_obstacle...");
  // NOTE: Under test mode, Y position is always 0 (not random)
  spawn_random_obstacle(obstacles, RIGHT);
  test_assert(obstacles->size() == 1);
  obstacle_t *spawned1 = obstacles->get(0);
  test_assert(spawned1->x == LCD_X_MIN);
  test_assert(spawned1->y == 0);
  spawn_random_obstacle(obstacles, LEFT);
  test_assert(obstacles->size() == 2);
  obstacle_t *spawned2 = obstacles->get(1);
  test_assert(spawned2->x == LCD_X_MAX);
  test_assert(spawned2->y == 0);
  free_all(obstacles);

  PRINTLN_FLASH("Testing collision_detected...");
  a = create_obstacle_at(5, 3, obstacles);
  b = create_obstacle_at(1, 0, obstacles);
  test_assert(collision_detected(obstacles, 5, 3));
  test_assert(collision_detected(obstacles, 1, 0));
  test_assert(!collision_detected(obstacles, 2, 0));
  free_all(obstacles);

  PRINTLN_FLASH("Testing create_obstacle_at...");
  a = create_obstacle_at(11, 8, obstacles);
  test_assert(a != NULL);
  test_assert(a->x == 11 && a->y == 8);
  test_assert(obstacles->get(0)->x == 11 && obstacles->get(0)->y == 8);
  free_all(obstacles);
}

#endif