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
 * Frees every obstacle in the given list, so that the list can then be
 * cleared without leaking memory.
 */
void free_all(LinkedPointerList<obstacle_t> *obstacles)
{
  obstacle_t *o;
  for(int i = 0; i < obstacles->size(); i++){
    o = obstacles->get(i);
    free(o);
  }
}

/**
 * Allocates a new obstacle and adds it onto the given obstacle list. Returns
 * a pointer to the created obstacle.
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