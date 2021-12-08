/*
   Testing framework - is currently a little gross

   To add a test, first add a struct with the desired start and expected end values of all the global variables into
   input_states and output_states (expect for all_obstacles, which you should set to NULL). Also add a millis for this
   test to take place in test_millis. Increase the size of all of the arrays and update num_tests.

   You will also need to setup your obstacle lists as desired. Add another "if" case to the add_objects function for the
   index of your test in the test arrays. Put obstacles in test_obstacles and test_exp_obstacles as desired.
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
} test_case_t;

//testing variables:
LinkedPointerList<obstacle_t> *test_obstacles; /* a list to initially set the obstacles for the test */
LinkedPointerList<obstacle_t> *test_exp_obstacles; /* a list containing the expected obstacles after the test */
// index 0 = NORMAL -> GAME_OVER
// index 1 = NORMAL -> PRE_DIRECTION_CHANGE
// index 2 = PRE_DIRECTION_CHANGE -> PRE_DIRECTION_CHANGE (also with obstacle speedup)
// index 3 = PRE_DIRECTION_CHANGE -> NORMAL (also with obstacle move)
// index 4 = PRE_DIRECTION_CHANGE -> GAME_OVER
/* values of global variables to start a test */
//state_vars_t input_states[5] = {
//  {false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
//  {false, false, 0, false, 1000, 29500, 19000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
//  {false, false, 29000, false, 1000, 29500, 29000, 26000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
//  {false, false, 29000, false, 30, 31900, 29000, 28000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
//  {false, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE}
//};
///* expected values of global variables after the test */
//state_vars_t output_states[5] = {
//  {false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 100, NULL, GAME_OVER},
//  {false, false, 30000, false, 1000, 29500, 30000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
//  {false, false, 29000, false, 950, 29500, 29000, 30000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
//  {false, false, 29000, false, 0, 32000, 29000, 28000, 0, 0, 1, 1, true, 0, 0, NULL, NORMAL},
//  {false, false, 29000, false, 1000, 29500, 29000, 30000, 0, 1, 1, 1, false, 0, 30000, NULL, GAME_OVER}
//};
test_case_t test_cases[5] = {
  {{false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   {false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 100, NULL, GAME_OVER}},
   
  {{false, false, 0, false, 1000, 29500, 19000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   {false, false, 30000, false, 1000, 29500, 30000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE}},
   
  {{false, false, 29000, false, 1000, 29500, 29000, 26000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   {false, false, 29000, false, 950, 29500, 29000, 30000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE}},
   
  {{false, false, 29000, false, 30, 31900, 29000, 28000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
    {false, false, 29000, false, 0, 32000, 29000, 28000, 0, 0, 1, 1, true, 0, 0, NULL, NORMAL}},
    
  {{false, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   {false, false, 29000, false, 1000, 29500, 29000, 30000, 0, 1, 1, 1, false, 0, 30000, NULL, GAME_OVER}}
};
/* fake times for the test to take place */
unsigned long test_millis[5] = {100, 30000, 30000, 32000, 30000};
int num_tests = 5; /* number of tests to run */

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
  //check if end state macthes expected
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
  test_passed = (test_passed and test_list(all_obstacles, expected_state.all_obstacles));
  return test_passed;
}

void add_objects(int test_num) {
  if (test_num == 0 || test_num == 4) {
    //creates a collision with player
    create_obstacle_at(1, 1, test_obstacles);
    create_obstacle_at(1, 1, test_exp_obstacles);
  }
  if (test_num == 3) {
    //creates a collision with player
    create_obstacle_at(5, 1, test_obstacles);
    create_obstacle_at(6, 1, test_exp_obstacles);
    create_obstacle_at(0, 0, test_exp_obstacles);
  }
}

void run_all_tests(void) {
  test_obstacles = new LinkedPointerList<obstacle_t>();
  test_exp_obstacles = new LinkedPointerList<obstacle_t>();

  for (int i = 0; i < num_tests; i++) {
    //setup the test
    test_case_t test_states = test_cases[i];
    state_vars_t input_state = test_states.input;
    state_vars_t output_state = test_states.output;
//    state_vars_t input_state = input_states[i];
//    state_vars_t output_state = output_states[i];
    add_objects(i);
    input_state.all_obstacles = test_obstacles;
    output_state.all_obstacles = test_exp_obstacles;
    //run the test
    if (!run_one_test(input_state, output_state, test_millis[i])) {
      Serial.println("failed a test!");
    }
    //clear the test
    free_all(test_obstacles); free_all(test_exp_obstacles);
  }

  test_obstacle_functions();
  test_joystick();

  Serial.println("Done with tests!");
}

#endif
