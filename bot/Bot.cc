#include "Bot.h"
#include "Score.h"

#include <limits.h>
#include <math.h>

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
    makeMoves();
    endTurn();
  }
}

// first pass: insanely inefficient, but re-call evaluate() from scratch for
// each potential ant move
// second pass: compute differentials for all ants, and find dependencies
// between ants to avoid recomputing differentials after each candidate ant move

// MAYBE TODO: use log(score) instead?
double Bot::iterateAnt(double bestscore, Ant &a)
{
  cerr << "moving ant @"<<a.pos_.col<<", "<<a.pos_.row<<endl;
  int bestmove = -1;
  for(int m=0;m<TDIRECTIONS;m++) {
    if(!a.Move(state, m))
      continue;
    double score = state.evalScore;
    cerr << "move("<<m<<") score="<<score<<endl;
    if(score > bestscore) {
      bestscore = score;
      bestmove = m;
    }
  }
  cerr << "using move "<<bestmove<<" for ant @"<<a.pos_.col<<", "<<a.pos_.row<<endl;
  a.Move(state, bestmove);
  return bestscore;
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
  cerr << "turn " << state.turn << ":" << endl;
  cerr << state << endl;

  double score = state.evalScore;

  state.dumpDistances();

  cerr << "eval_init = " << score << endl;
  for(size_t i=0;i<state.myAnts.size();i++) {
    score = iterateAnt(score, state.myAnts[i]);
    cerr << "eval(" << i << ") = " << score << endl;
  }

  for(size_t i=0;i<state.myAnts.size();i++) {
    state.myAnts[i].CommitMove(state);
  }

  cerr << "time taken: " << state.timer.getTime() << "ms" << endl << endl;
}

//finishes the turn
void Bot::endTurn()
{
  if(state.turn > 0)
    state.reset();
  state.turn++;

  cout << "go" << endl;
}
