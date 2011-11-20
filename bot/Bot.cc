#include "Ant.h"
#include "Bot.h"
#include "Score.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

using namespace std;

static const int kNMaximizePasses = 1;

//constructor
Bot::Bot()
{
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
    state.updateVisionInformation();
    state.updateStateEstimate();
    state.updateDistanceInformation();
    // FIXME: this is a disaster
    for(int a=0; a<(int) state.myAnts.size(); a++) {
      state.myAnts[a]->UpdateScore(state);
    }
    for(int a=0; a<(int) state.enemyAnts.size(); a++) {
      state.enemyAnts[a]->UpdateScore(state);
    }
    fprintf(stderr, "initial eval score: %g\n", state.evalScore);
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
  // FIXME: time this stuff
  int Nmy = state.myAnts.size();
  int Nenemy = state.enemyAnts.size();
  int Nants = Nmy + Nenemy;
  int maxTurnTime = std::min(475.0, 95*state.turntime/100);
  for(int smp=0;smp<5000;smp++) {
    for(int j=0;j<Nants;j++) {
      int i = lrand48()%Nants;
      if(i < Nmy)
        state.myAnts[i]->GibbsStep(state);
      else
        state.enemyAnts[i-Nmy]->GibbsStep(state);
    }
    if(state.timer.getTime() > maxTurnTime)
      break;
  }

  for(int j=0;j<kNMaximizePasses;j++)
    for(size_t i=0;i<state.myAnts.size();i++)
      state.myAnts[i]->MaximizeMove(state);

  for(size_t i=0;i<state.myAnts.size();i++)
    state.myAnts[i]->CommitMove(state);

  cerr << "time taken: " << state.timer.getTime() << "ms; score=" << state.evalScore << endl << endl;
}

//finishes the turn
void Bot::endTurn()
{
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
}
