double t;
int init(double l, double ldot) {
  t = 0;
  return 0; 
}

void clean_up(){
}

int rl_alg(double l, double ldot, double r, int is_terminal) {
  t += 0.1;
  return sin(t) > 0; 
}

