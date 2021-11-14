/*
   obstacles.ino
   Functions for managing / updating obstacles.
*/

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
        error("invalid direction in move_obstacles");
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
  uint8_t y = rand() % (LCD_Y_MAX + 1);
  uint8_t x;

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
      error("invalid direction in spawn_random_obstacle");
  }

  // Add the new obstacle
  obstacle_t *obs = (obstacle_t *)malloc(sizeof(obstacle_t));
  obs->x = x;
  obs->y = y;
  obstacles->add(obs);
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
