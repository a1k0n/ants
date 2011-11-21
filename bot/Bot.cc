#include "Ant.h"
#include "Bot.h"
#include "Score.h"

#include <limits.h>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;

static const int kNMaximizePasses = 1;

// {{{ run timing
long _get_time()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec + tv.tv_sec*1000000;
}

static long _timer, _timeout;
static volatile bool _timed_out = false;

static void _alrm_handler(int sig) { _timed_out = true; }

static void reset_timer(long t)
{
  _timer = _get_time();
  itimerval timer;
  memset(&timer, 0, sizeof(timer));
  timer.it_value.tv_sec = t/1000000;
  timer.it_value.tv_usec = t%1000000;
  setitimer(ITIMER_REAL, &timer, NULL);
  _timed_out = false;
  _timeout = t;
}

static long elapsed_time() { return _get_time() - _timer; }
// }}}

//constructor
Bot::Bot()
{
  signal(SIGALRM, _alrm_handler);
}

//plays a single game of Ants.
void Bot::playGame()
{
  //reads the game parameters and sets up
  cin >> state;
  state.setup();
  endTurn();

  //continues making moves while the game is not over
  while(cin >> state)
  {
    int maxTurnTime = std::min(475.0, 95*state.turntime/100);
    reset_timer(1000*maxTurnTime);
    state.updateVisionInformation();
    state.updateStateEstimate();
    state.updateDistanceInformation();

    for(int a=0; a<(int) state.myAnts.size(); a++) {
      state.myAnts[a]->UpdateScore(state);
    }
    for(int a=0; a<(int) state.enemyAnts.size(); a++) {
      state.enemyAnts[a]->UpdateScore(state);
    }
    makeMoves();
    endTurn();
  }
}

// set up dependency chain like this:
// uuuur
// luurr  u: dy < 0; dy <= dx < -dy
// ll*rr  l: dx < 0; dx < dy <= -dx
// llddr
// ldddd
#ifdef CONDITIONAL
static void AssignConditionalDependencies(vector<Ant*> &ants)
{
  for(size_t i=0;i<ants.size();i++) {
    Ant *closest_above = NULL, *closest_left = NULL;
    int dist_above = INT_MAX, dist_left = INT_MAX;
    for(size_t j=0;j<ants.size();j++) {
      if(j == i)
        continue;
      int dx = (ants[j]->pos_.col -
                ants[i]->pos_.col),
          dy = (ants[j]->pos_.row -
                ants[i]->pos_.row);
#if 0
      fprintf(stderr, "ant(%d) <-> ant(%d): dx=%d dy=%d\n", i, j, dx, dy);
#endif
      int dist = dx*dx + dy*dy;
      if(dx < 0 && dx < dy && dy <= -dx && dist < dist_left) {
        closest_left = ants[j];
        dist_left = dist;
      }
      if(dy < 0 && dy <= dx && dx < -dy && dist < dist_above) {
        closest_above = ants[j];
        dist_above = dist;
      }
    }
    ants[i]->dependUp_ = closest_above;
    ants[i]->dependLeft_ = closest_left;
#ifdef BLAH
    fprintf(stderr, "ant (%d,%d) cond.deps: u:(%d,%d) l:(%d,%d)\n",
            ants[i]->pos_.col, ants[i]->pos_.row,
            closest_above ? closest_above->pos_.col : -1,
            closest_above ? closest_above->pos_.row : -1,
            closest_left ? closest_left->pos_.col : -1,
            closest_left ? closest_left->pos_.row : -1);
#endif
  }
}
#endif

//makes the bots moves for the turn
void Bot::makeMoves()
{
  fprintf(stderr, "turn %d:\n", state.turn);
  cerr << state << endl;

#ifdef VERBOSE
  state.dumpDistances(Square::DIST_MY_ANTS);
#endif

#ifdef CONDITIONAL
  AssignConditionalDependencies(state.myAnts);
  AssignConditionalDependencies(state.enemyAnts);
#endif

  for(size_t i=0;i<state.myAnts.size();i++)
    state.myAnts[i]->ComputeDeltaScores(state);

  fprintf(stderr, "initial score deltas computed; evalScore=%g\n", state.evalScore);

  int Nmy = state.myAnts.size();
  int Nenemy = state.enemyAnts.size();
  int Nants = Nmy + Nenemy;
  int Nsamples = 5000*Nants;
  int smp = 0;
  for(smp=0;smp<Nsamples && !_timed_out;smp++) {
    int i = lrand48()%Nants;
    if(i < Nmy)
      state.myAnts[i]->GibbsStep(state);
    else
      state.enemyAnts[i-Nmy]->GibbsStep(state);
  }

  for(int j=0;j<kNMaximizePasses;j++)
    for(size_t i=0;i<state.myAnts.size();i++)
      state.myAnts[i]->MaximizeMove(state);

  for(size_t i=0;i<state.myAnts.size();i++)
    state.myAnts[i]->CommitMove(state);

  cerr << smp << " move samples in " << elapsed_time() << "us\n";
}

//finishes the turn
void Bot::endTurn()
{
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
}
