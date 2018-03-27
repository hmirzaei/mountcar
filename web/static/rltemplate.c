// initialization function
// l: initial position
// ldot: initial speed
// return: initial action
int init(double l, double ldot) {
  // write your initialization code here
  return 0; // return initial action
}

// clean up function
void clean_up(){
  // write the clean up code here, e.g. free memory, etc.
}

// RL step function
// l: current position
// ldot: current speed
// r: reward for previous action
// is_terminal: is the episode ended, i.e. goal has been reached
// return: calculated action
int rl_alg(double l, double ldot, double r, int is_terminal) {
  // write the RL algorithm here
  return 0; // return algorithm output action for this state
}
