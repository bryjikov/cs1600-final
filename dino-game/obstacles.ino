/*
 * obstacles.ino
 * Functions for managing / updating obstacles.
 */

#define X_MIN 0
#define X_MAX 19
#define Y_MIN 0
#define Y_MAX 3

/*
   Moves each obstacle in a list 1 unit in the indicated direction.
*/
void move_obstacles(LinkedList<obstacle_t> *obstacles, direction_t dir)
{
  obstacle_t obs;
  for (int i = 0; i < obstacles->size(); i++) {
    obs = obstacles->get(i);

    switch (dir) {
      case LEFT:
        obs.x -= 1;
        break;
      case RIGHT:
        obs.x += 1;
        break;
      default:
        error("invalid direction in move_obstacles");
    }

    obstacles->set(i, obs);
  }
}

/*
   Removes any obstacles from the given list that are outside
   the bounds of the screen.
*/
void remove_out_of_bounds(LinkedList<obstacle_t> *obstacles)
{
  obstacle_t obs;
  LinkedList<obstacle_t> only_in_bounds;

  for (int i = 0; i < obstacles->size(); i++) {
    obs = obstacles->get(i);

    // Only keep obstacles within bounds
    if (obs.x >= X_MIN && obs.x <= X_MAX && obs.y >= Y_MIN && obs.y <= Y_MAX)
    {
      only_in_bounds.add(obs);
    }
  }

  // Replace original list with filtered list
  obstacles->clear();
  for (int i = 0; i < only_in_bounds.size(); i++) {
    obstacles->add(only_in_bounds.get(i));
  }
}

/**
 * Generates a new obstacle at a random Y position and
 * adds it to the given list of obstacles. The current
 * direction is indicated so that obstacles can be spawned.
 */
void spawn_random_obstacle(LinkedList<obstacle_t> *obstacles, direction_t dir)
{
  uint8_t y = rand() % (Y_MAX + 1);
  uint8_t x;

  // If obstacles are moving left, spawn on the right (X_MAX)
  // If obstacles are moving right, spawn on the left (X_MIN)
  switch (dir) {
    case LEFT:
      x = X_MAX;
      break;
    case RIGHT:
      x = X_MIN;
      break;
    default:
      error("invalid direction in spawn_random_obstacle");
  }

  // Add the new obstacle
  obstacle_t obs = { x, y };
  obstacles->add(obs);
}
