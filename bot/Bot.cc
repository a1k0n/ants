#include "Bot.h"
#include "Score.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>

using namespace std;

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
      state.evalScore += AntScore(state, state.myAnts[a]);
    }
    makeMoves();
    endTurn();
  }
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
  fprintf(stderr, "turn %d:\n", state.turn);
  cerr << state << endl;

  double score = state.evalScore;

#ifdef VERBOSE
  state.dumpDistances(Square::DIST_MY_ANTS);
#endif

  for(size_t i=1;i<state.myAnts.size();i++) {
    // FIXME: come up with an allocation scheme
    state.myAnts[i]->dependency_ = state.myAnts[i-1];
  }

  for(size_t i=0;i<state.myAnts.size();i++)
    state.myAnts[i]->ComputeDeltaScores(state);

  // FIXME: time this stuff
  for(int smp=0;smp<1000;smp++) {
    for(size_t i=0;i<state.myAnts.size();i++)
      state.myAnts[i]->GibbsStep(state);
    for(size_t i=0;i<state.enemyAnts.size();i++)
      state.enemyAnts[i]->GibbsStep(state);
    if(state.timer.getTime() > 300)
      break;
  }

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
