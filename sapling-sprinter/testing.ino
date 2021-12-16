/*
   Testing framework - is currently a little gross
   To add a test, first add a struct with the desired start and expected end values of all the global variables into
   input_states and output_states and test_millis (expect for all_obstacles, which you should set to NULL). Increase the size of all of the arrays and update num_tests.
   You will also need to setup your obstacle lists as desired. Add another "if" case to the add_objects function for the
   index of your test in the test arrays. Put obstacles in test_obstacles and test_exp_onbstacles as desired.
*/

// Only include this code if we are in testing mode.
#ifdef TESTING

typedef struct state_vars {
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
  state_t current_state;                   /* state of the fsm */
} state_vars_t;

typedef struct test_case {
  state_vars_t input;
  state_vars_t output;
  unsigned long mils;
} test_case_t;

//testing variables:
LinkedPointerList<obstacle_t> *test_obstacles; /* a list to initially set the obstacles for the test */
LinkedPointerList<obstacle_t> *test_exp_obstacles; /* a list containing the expected obstacles after the test */

const test_case_t test_cases[] PROGMEM = {
  // TESTS THAT HAVE OBSTACLES
  { {false, false, 0, false, 100, 50, 95, 90, 0, RIGHT, 1, 1, false, 0, 0, NULL, NORMAL},
    {true, false, 0, false, 100, 50, 95, 90, 0, RIGHT, 1, 1, false, 0, 100, NULL, NORMAL},
    100
  },

  { {false, false, 29000, false, 0, 31900, 29000, 31000, 0, RIGHT, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    {false, false, 29000, false, 0, 31900, 29000, 31000, 0, LEFT, 1, 1, false, 0, 0, NULL, NORMAL},
    32000
  },

  { {false, false, 29000, false, 1000, 29500, 29000, 29000, 0, RIGHT, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    {true, false, 29000, false, 1000, 29500, 29000, 29000, 0, RIGHT, 1, 1, false, 0, 30000, NULL, PRE_DIRECTION_CHANGE},
    30000
  },

  { {true, false, 0, false, 100, 50, 95, 90, 0, RIGHT, 1, 1, false, 0, 100, NULL, NORMAL},
    {false, false, 0, false, 100, 50, 95, 90, 0, RIGHT, 1, 1, false, 0, 100, NULL, GAME_OVER},
    100
  },

  { {false, false, 0, false, 800, 12000, 11000, 13000, 0, RIGHT, 5, 6, false, 0, 0, NULL, NORMAL},
    {false, false, 0, false, 800, 14000, 11000, 13000, 0, RIGHT, 5, 6, false, 0, 0, NULL, NORMAL},
    14000
  },

  { {true, false, 29000, false, 1000, 29500, 29000, 29000, 0, RIGHT, 1, 1, false, 0, 30000, NULL, PRE_DIRECTION_CHANGE},
    {false, false, 29000, false, 1000, 29500, 29000, 29000, 0, RIGHT, 1, 1, false, 0, 30000, NULL, GAME_OVER},
    30000
  },

  // TESTS THAT DO NOT HAVE OBSTACLES
  { {false, false, 0, false, 0, 0, 0, 0, 0, LEFT, 0, 0, false, 0, 0, NULL, SETUP},
    {false, false, 0, false, 0, 0, 0, 0, 0, LEFT, 0, 0, false, 0, 0, NULL, SETUP},
    1999
  },

  { {false, false, 0, false, 0, 0, 0, 0, 0, LEFT, 0, 0, false, 0, 0, NULL, SETUP},
    {false, false, 0, false, 1000, 0, 0, 0, 0, LEFT, 10, 3, true, 2000, 0, NULL, NORMAL},
    2000
  },

  { {false, false, 0, false, 0, 0, 0, 0, 1000, LEFT, 0, 0, false, 0, 0, NULL, SETUP},
    {false, false, 0, false, 1000, 0, 0, 0, 0, LEFT, 10, 3, true, 3000, 0, NULL, NORMAL},
    3000
  },

  { {false, false, 0, false, 1000, 29500, 19000, 29000, 0, RIGHT, 1, 1, false, 0, 0, NULL, NORMAL},
    {false, true, 30000, false, 1000, 29500, 30000, 29000, 0, RIGHT, 1, 1, false, 0, 0, NULL, NORMAL},
    30000
  },

  { {false, true, 30000, false, 1000, 29500, 30000, 29000, 0, RIGHT, 1, 1, false, 0, 0, NULL, NORMAL},
    {false, false, 30000, false, 1000, 29500, 30000, 29000, 0, RIGHT, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    30001
  },

  { {false, false, 29000, false, 1000, 29500, 29000, 26000, 0, RIGHT, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    {false, false, 29000, false, 950, 29500, 29000, 30000, 0, RIGHT, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    30000
  },

  { {false, false, 14000, false, 500, 13800, 13800, 13800, 0, LEFT, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
    {false, false, 14000, false, 500, 13800, 13800, 13800, 0, LEFT, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
    14000
  },

  { {false, false, 14000, true, 500, 13800, 13800, 13800, 0, RIGHT, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
    {false, false, 14000, false, 500, 13800, 13800, 13800, 14000, RIGHT, 10, 1, false, 2000, 12000, NULL, SETUP},
    14000
  }
};

int num_tests = sizeof(test_cases) / sizeof(test_case_t); /* number of tests to run */

bool run_one_test(state_vars_t start_state, state_vars_t expected_state, unsigned long current_millis);

bool test_list(LinkedPointerList<obstacle_t> *actual_obstacles, LinkedPointerList<obstacle_t> *expected_obstacles) {
  if (actual_obstacles->size() != expected_obstacles->size()) {
    return false;
  }

  obstacle_t* a;
  obstacle_t* e;
  for (int i = 0; i < actual_obstacles->size(); i++) {
    a = actual_obstacles->get(i);
    e = expected_obstacles->get(i);
    if (a->x != e->x || a->y != e->y) {
      return false;
    }
  }

  return true;
}

bool run_one_test(state_vars_t start_state, state_vars_t expected_state, unsigned long current_millis) {
  //set global variables to match desired input
  game_over_flag = start_state.game_over_flag;
  pre_direction_change_flag = start_state.pre_direction_change_flag;
  time_entered_pdc = start_state.time_entered_pdc;
  restart_flag = start_state.restart_flag;
  obstacle_move_interval = start_state.obstacle_move_interval;
  time_last_obstacle_move = start_state.time_last_obstacle_move;
  time_last_dir_chg = start_state.time_last_dir_chg;
  time_last_speed_up = start_state.time_last_speed_up;
  time_entered_setup = start_state.time_entered_setup;
  obstacle_direction = start_state.obstacle_direction;
  player_x = start_state.player_x;
  player_y = start_state.player_y;
  moved = start_state.moved;
  start_time = start_state.start_time;
  duration = start_state.duration;
  all_obstacles = start_state.all_obstacles;
  current_state = start_state.current_state;

  // run fsm
  state_t end_state = update_game_state(current_millis);

  //check if end state matches expected
  bool test_passed = (end_state == expected_state.current_state and
                      game_over_flag == expected_state.game_over_flag and
                      pre_direction_change_flag == expected_state.pre_direction_change_flag and
                      time_entered_pdc == expected_state.time_entered_pdc and
                      restart_flag == expected_state.restart_flag and
                      obstacle_move_interval == expected_state.obstacle_move_interval and
                      time_last_obstacle_move == expected_state.time_last_obstacle_move and
                      time_last_dir_chg == expected_state.time_last_dir_chg and
                      time_last_speed_up == expected_state.time_last_speed_up and
                      time_entered_setup == expected_state.time_entered_setup and
                      obstacle_direction == expected_state.obstacle_direction and
                      player_x == expected_state.player_x and
                      player_y == expected_state.player_y and
                      moved == expected_state.moved and
                      start_time == expected_state.start_time and
                      duration == expected_state.duration);

  bool obstacles_match = test_list(all_obstacles, expected_state.all_obstacles);

  return test_passed and obstacles_match;
}

void add_objects(int test_num) {
  if (test_num == 0 || test_num == 2) {
    //creates a collision with player
    create_obstacle_at(1, 1, test_obstacles);
    create_obstacle_at(1, 1, test_exp_obstacles);
  }
  if (test_num == 1) {
    //creates a collision with player
    create_obstacle_at(5, 1, test_obstacles);
    create_obstacle_at(5, 1, test_exp_obstacles);
    //create_obstacle_at(0, 0, test_exp_obstacles);
  }
  if (test_num == 4) {
    //moves obstacle during transition
    create_obstacle_at(0, 0, test_exp_obstacles);
  }
}

void run_all_tests(void) {
  test_obstacles = new LinkedPointerList<obstacle_t>();
  test_exp_obstacles = new LinkedPointerList<obstacle_t>();

  serial_printf("Number of FSM tests to run: %d\n", num_tests);

  for (int i = 0; i < num_tests; i++) {
    //setup the test
    state_vars_t input_state, output_state;
    memcpy_P(&input_state, &(test_cases[i].input), sizeof(state_vars_t));
    memcpy_P(&output_state, &(test_cases[i].output), sizeof(state_vars_t));
    unsigned long test_mils;
    memcpy_P(&test_mils, &(test_cases[i].mils), sizeof(unsigned long));

    add_objects(i);
    input_state.all_obstacles = test_obstacles;
    output_state.all_obstacles = test_exp_obstacles;

    //run the test
    if (!run_one_test(input_state, output_state, test_mils)) {
      Serial.print("FSM TEST FAILED: TEST ");
    } else {
      Serial.print("fsm test passed: test ");
    }
    Serial.print(i + 1);
    Serial.print("/");
    Serial.println(num_tests);

    //clear the test
    free_all(test_obstacles); free_all(test_exp_obstacles);
  }

  test_obstacle_functions();
  test_joystick();

  Serial.println("Done with tests!");
}

#endif
