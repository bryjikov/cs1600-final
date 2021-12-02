typedef struct {
  bool game_over_flag;                     /* Set when the player has collided with an obstacle */
  bool pre_direction_change_flag;          /* Set when a direction change is upcoming and user should be warned */
  unsigned long time_entered_pdc;          /* Time (ms) at which the pre direction change state was entered */
  bool restart_flag;                       /* Set when the game should start over */
  unsigned obstacle_move_interval;         /* How often (ms) obstacles move */
  unsigned long time_last_obstacle_move;   /* Time (ms) of last obstacle movement */
  unsigned long time_last_dir_chg;         /* Time (ms) of last direction change event */
  unsigned long time_last_speed_up;        /* Time (ms) that obstacles were last sped up */
  unsigned long time_entered_setup;        /* Time (ms) when SETUP state was entered */
  direction_t obstacle_direction;          /* Direction of movement for obstacles */
  int player_x;                            /* Current X position of the player */
  int player_y;                            /* Current Y position of the player */
  bool moved;                              /* Set if either the player or obstacles moved since the last display */
  unsigned long start_time;                /* The time (in milis) when the player begins */
  unsigned long duration;                  /* The total time that the game lasted */
  LinkedPointerList<obstacle_t> *all_obstacles;     /* List containing all currently active obstacles */
} state_vars;

​
