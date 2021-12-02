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
  state_t current_state;
} state_vars;



bool run_one_test(state_vars start_state, state_vars expected_state, unsigned long current_millis){
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
                      duration == expected_state.duration and                  
                      all_obstacles == expected_state.all_obstacles);

   return test_passed;                      
}

​
