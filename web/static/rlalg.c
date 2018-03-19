#define NUM_STATES 2
#define NUM_ACTS 2
#define NUM_RET_ARRAY 1000
#define NUM_GRIDS 20
#define NUM_TILINGS 20
#define EPS 0.05
#define ALPHA 0.05
#define GAMMA 1.0

double STATE_BOUNDS[NUM_STATES][2], GRID_WIDTH[NUM_STATES];
int NUM_FEATS;
double ** TILING_OFFSETS, ** params;;
double prev_l, prev_ldot;
int prev_a;

int ipow(double base, int exp)
{
  int result = 1;
  while (exp) {
    if (exp & 1) result *= base;
    exp >>= 1;
    base *= base;
  }
  return result;
}

double fRand(double fMin, double fMax)
{
  double f = (double)rand() / RAND_MAX;
  return fMin + f * (fMax - fMin);
}

void features(double * s, int * fa) {
  int i, j;
  int tilingIndexes [NUM_TILINGS][NUM_STATES];
  for (i = 0; i< NUM_TILINGS; i++) 
    for (j = 0; j< NUM_STATES; j++) 
      tilingIndexes[i][j] = 
	fmin(fmax((int)((s[j] - STATE_BOUNDS[j][0] - TILING_OFFSETS[i][j]) 
			/ GRID_WIDTH[j]), 0), NUM_GRIDS - 1);

  for (i = 0; i< NUM_TILINGS; i++) {
    fa[i] = ipow(NUM_GRIDS, NUM_STATES) * i;
    for (j = 0; j< NUM_STATES; j++) 
      fa[i] += ipow(NUM_GRIDS, j) * tilingIndexes[i][j];
  }
}

double Q(int * fa, int a){
  double * tmp = params[a];
  double  sum = 0;
  int i;
  for(i = 0; i < NUM_TILINGS; i++) sum += tmp[fa[i]];
  return sum;
}

void greedy_policy(double * s, int * a, double * v){
  int fa[NUM_TILINGS];
  features(s, fa);
  double value = -DBL_MAX;
  int indecies[NUM_ACTS];
  int indLen = 0, i;
  for (i=0; i<NUM_ACTS; i++) {
    double tmp = Q(fa, i);
    if (tmp > value) {
      value = tmp;
      indLen = 0;
      indecies[indLen++] = i;
    } else if (tmp == value)
      indecies[indLen++] = i;
  }
  *a = indecies[rand() % indLen];
  *v = value;
}

void eps_policy(double * s, int * a, double * v){   
  int greedy_action;
  double value;
  greedy_policy(s, &greedy_action, &value);
  if (rand() < (int)(EPS * RAND_MAX)) {
    *a = rand() % NUM_ACTS;
    *v = value; // dummy value not correct though
  } else {
    *a = greedy_action;
    *v = value;
  }  
}

int init(double l, double ldot) {
  int i, j;
  TILING_OFFSETS =(double **) malloc(NUM_TILINGS * sizeof(double *));  
  for(i = 0; i < NUM_TILINGS; ++i)
    TILING_OFFSETS[i]=(double *) malloc(NUM_STATES * sizeof(double));
  
  double temp[NUM_STATES][2] = {{-130, 130}, {-800, 800}};
  memcpy(STATE_BOUNDS, temp, NUM_STATES*2*sizeof(double));

  NUM_FEATS = ipow(NUM_GRIDS, NUM_STATES) * NUM_TILINGS;
  for (i = 0; i<NUM_STATES; i++) {
    GRID_WIDTH[i] = 
      (double)(STATE_BOUNDS[i][1] - STATE_BOUNDS[i][0]) / NUM_GRIDS;
  }
  srand (1);
  for (i = 0; i<NUM_TILINGS; i++)
    for (j = 0; j<NUM_STATES; j++)
      TILING_OFFSETS[i][j] = -fRand(-GRID_WIDTH[j] / 2, GRID_WIDTH[j]/ 2);

  params =(double **) malloc(NUM_ACTS * sizeof(double *));  
  for(i = 0; i < NUM_ACTS; ++i) {
    params[i]=(double *) malloc(NUM_FEATS * sizeof(double));
    memset(params[i], 0, NUM_FEATS * sizeof(double));
  }
  prev_l = l; prev_ldot = ldot; prev_a = 1;
  return prev_a;
}

void clean_up(){
  int i;
  for(i = 0; i < NUM_TILINGS; ++i) {
    free(TILING_OFFSETS[i]);
  }
  free(TILING_OFFSETS);
  for(i = 0; i < NUM_ACTS; ++i) {
    free(params[i]);
  }
  free(params);
}

int rl_alg(double l, double ldot, double r, int is_terminal) {
  double sa[NUM_STATES];
  double sa2[NUM_STATES];
  sa[0] = prev_l;
  sa[1] = prev_ldot;
  prev_l = l;
  prev_ldot = ldot; 
  sa2[0] = l;
  sa2[1] = ldot;

  int i, a;
  double Q_a;
  eps_policy(sa, &a, &Q_a);
  int fa[NUM_TILINGS];
  features(sa, fa);

  double delta = r - Q(fa, prev_a);
  double * tmp = params[prev_a];
  if (is_terminal) {
    for(i = 0; i < NUM_TILINGS; i++)
      tmp[fa[i]] += ALPHA * delta;
  } else {
    int ap;
    double Q_ap;
    greedy_policy(sa2, &ap, &Q_ap);
    delta += GAMMA * Q_ap;
    for(i = 0; i < NUM_TILINGS; i++)
      tmp[fa[i]] += ALPHA * delta;
  }
  prev_a = a;
  return a;
}

