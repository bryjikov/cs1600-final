/*
   Testing framework - is currently a little gross
   To add a test, first add a struct with the desired start and expected end values of all the global variables into
   input_states and output_states and test_millis (expect for all_obstacles, which you should set to NULL). Increase the size of all of the arrays and update num_tests.
   You will also need to setup your obstacle lists as desired. Add another "if" case to the add_objects function for the
   index of your test in the test arrays. Put obstacles in test_obstacles and test_exp_onbstacles as desired.
*/

// Only include this code if we are in testing mode.
#ifdef TESTING

//testing variables:
LinkedPointerList<obstacle_t> *test_obstacles; /* a list to initially set the obstacles for the test */
LinkedPointerList<obstacle_t> *test_exp_obstacles; /* a list containing the expected obstacles after the test */

const test_case_t test_cases[] PROGMEM = {
  // TESTS THAT HAVE OBSTACLES
  {{false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   {true, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 100, NULL, NORMAL},
   100},

  {{false, false, 29000, false, 0, 31900, 29000, 31000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   {false, false, 29000, false, 0, 31900, 29000, 31000, 0, 0, 1, 1, false, 0, 0, NULL, NORMAL},
   32000},

  {{false, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   {true, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 30000, NULL, PRE_DIRECTION_CHANGE},
   30000},

  {{true, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 100, NULL, NORMAL},
   {false, false, 0, false, 100, 50, 95, 90, 0, 1, 1, 1, false, 0, 100, NULL, GAME_OVER},
   100},

  {{false, false, 0, false, 800, 12000, 11000, 13000, 0, 1, 5, 6, false, 0, 0, NULL, NORMAL},
   {false, false, 0, false, 800, 14000, 11000, 13000, 0, 1, 5, 6, false, 0, 0, NULL, NORMAL},
   14000},

  {{true, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 30000, NULL, PRE_DIRECTION_CHANGE},
   {false, false, 29000, false, 1000, 29500, 29000, 29000, 0, 1, 1, 1, false, 0, 30000, NULL, GAME_OVER},
   30000},

  // TESTS THAT DO NOT HAVE OBSTACLES
  {{false, false, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, NULL, SETUP},
   {false, false, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, NULL, SETUP},
   1999},
  
  {{false, false, 0, false, 0, 0, 0, 0, 0, 0, 0, 0, false, 0, 0, NULL, SETUP},
   {false, false, 0, false, 1000, 0, 0, 0, 0, 0, 10, 3, true, 2000, 0, NULL, NORMAL},
   2000},

  {{false, false, 0, false, 0, 0, 0, 0, 1000, 0, 0, 0, false, 0, 0, NULL, SETUP},
   {false, false, 0, false, 1000, 0, 0, 0, 0, 0, 10, 3, true, 3000, 0, NULL, NORMAL},
   3000},
  
  {{false, false, 0, false, 1000, 29500, 19000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   {false, true, 30000, false, 1000, 29500, 30000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   30000},

  {{false, true, 30000, false, 1000, 29500, 30000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, NORMAL},
   {false, false, 30000, false, 1000, 29500, 30000, 29000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   30001},

  {{false, false, 29000, false, 1000, 29500, 29000, 26000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   {false, false, 29000, false, 950, 29500, 29000, 30000, 0, 1, 1, 1, false, 0, 0, NULL, PRE_DIRECTION_CHANGE},
   30000},

  {{false, false, 14000, false, 500, 13800, 13800, 13800, 0, 0, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
   {false, false, 14000, false, 500, 13800, 13800, 13800, 0, 0, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
   14000},

  {{false, false, 14000, true, 500, 13800, 13800, 13800, 0, 1, 10, 1, false, 2000, 12000, NULL, GAME_OVER},
   {false, false, 14000, false, 500, 13800, 13800, 13800, 14000, 1, 10, 1, false, 2000, 12000, NULL, SETUP},
   14000}
};

int num_tests = sizeof(test_cases) / sizeof(test_case_t); /* number of tests to run */

bool run_one_test(test_case_t test_case);

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

bool run_one_test(test_case_t *test_case) {
  state_vars_t start_state = test_case->input;
  state_vars_t expected_state = test_case->output;

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
  state_t end_state = update_game_state(test_case->mils);
//  Serial.println("Printing State: ");
//  Serial.print("game_over_flag: ");
//  Serial.println(game_over_flag);
//  Serial.print("pre_direction_change_flag: ");
//  Serial.println(pre_direction_change_flag);
//  Serial.print("time_entered_pdc: ");
//  Serial.println(time_entered_pdc);
//  Serial.print("restart_flag: ");
//  Serial.println(restart_flag);
//  Serial.print("obstacle_move_interval: ");
//  Serial.println(obstacle_move_interval);
//  Serial.print("time_last_obstacle_move: ");
//  Serial.println(time_last_obstacle_move);
//  Serial.print("time_last_dir_chg: ");
//  Serial.println(time_last_dir_chg);
//  Serial.print("time_last_speed_up: ");
//  Serial.println(time_last_speed_up);
//  Serial.print("time_entered_setup: ");
//  Serial.println(time_entered_setup);
//  Serial.print("obstacle_direction: ");
//  Serial.println(obstacle_direction);
//  Serial.print("player_x: ");
//  Serial.println(player_x);
//  Serial.print("player_y: ");
//  Serial.println(player_y);
//  Serial.print("moved: ");
//  Serial.println(moved);
//  Serial.print("start_time: ");
//  Serial.println(start_time);
//  Serial.print("duration: ");
//  Serial.println(duration);
//  Serial.print("end_state: ");
//  Serial.println(end_state);
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

//  serial_printf("all_obstacles size: %d\n", all_obstacles->size());
//  serial_printf("expected obstacles size: %d\n", expected_state.all_obstacles->size());
  
  bool obstacles_match = test_list(all_obstacles, expected_state.all_obstacles);

  serial_printf("obstacles match? %d\n", obstacles_match);
  serial_printf("test passed? %d\n", test_passed); 

  bool mocks_match = (
    mock_led_value == test_case->mock_led_value and
    mock_displayed_player_x == test_case->mock_displayed_player_x and
    mock_displayed_player_y == test_case->mock_displayed_player_y and
    mock_num_display_obstacle_calls == test_case->mock_num_display_obstacle_calls and
    mock_setup_was_displayed == test_case->mock_setup_was_displayed and
    mock_game_over_was_displayed == test_case->mock_game_over_was_displayed
  );
  
  return test_passed and obstacles_match and mocks_match; 
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

  for (int i = 0; i < num_tests; i++) {
    //setup the test
    test_case_t test_case;
    memcpy_P(&test_case, &test_cases[i], sizeof(test_case_t));

    add_objects(i);
    test_case.input.all_obstacles = test_obstacles;
    test_case.output.all_obstacles = test_exp_obstacles;
    //run the test
    if (!run_one_test(&test_case)) {
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
