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

// first pass: insanely inefficient, but re-call evaluate() from scratch for
// each potential ant move
// second pass: compute differentials for all ants, and find dependencies
// between ants to avoid recomputing differentials after each candidate ant move

double coinflip(double prob)
{
  return drand48() < prob;
}

// MAYBE TODO: use log(score) instead?
double Bot::iterateAnt(double bestscore, Ant *a)
{
  Location orig_pos = a->pos_.prev(a->move_);
#ifdef VERBOSE
  fprintf(stderr, "moving ant @%d,%d\n", orig_pos.col, orig_pos.row);
#endif
  int bestmove = a->move_;
  int nequal = 1;
  for(int m=0;m<TDIRECTIONS;m++) {
    if(!a->Move(state, m))
      continue;
    double score = state.evalScore;
#ifdef VERBOSE
    cerr << "move("<<m<<") score="<<score<<" dead_="<<a->dead_<<endl;
    a->dumpEnemies();
#endif
    if(score > bestscore) {
      bestscore = score;
      bestmove = m;
      nequal = 1;
    } else if(score == bestscore) {
      nequal++;
      if(coinflip(1.0/nequal))
        bestmove = m;
    }
  }
  a->Move(state, bestmove);
#ifdef VERBOSE
  fprintf(stderr, "using move %d for ant @%d,%d (score should be %g, is now %g)\n",
          bestmove, orig_pos.col, orig_pos.row, bestscore, state.evalScore);
#endif
  bestscore = state.evalScore;
  return bestscore;
}

double Bot::iterateEnemyAnt(double worstscore, Ant *a)
{
  Location orig_pos = a->pos_.prev(a->move_);
#ifdef VERBOSE
  fprintf(stderr, "moving enemy ant @%d,%d\n", orig_pos.col, orig_pos.row);
#endif
  int worstmove = a->move_;
  int nequal = 1;
  for(int m=0;m<TDIRECTIONS;m++) {
    if(!a->Move(state, m))
      continue;
    double score = state.evalScore;
#ifdef VERBOSE
    cerr << "move("<<m<<") score="<<score<<" dead_="<<a->dead_<<endl;
#endif
    if(score < worstscore) {
      worstscore = score;
      worstmove = m;
      nequal = 1;
    } else if(score == worstscore) {
      nequal++;
      if(coinflip(1.0/nequal))
        worstmove = m;
    }
  }
  a->Move(state, worstmove);
#ifdef VERBOSE
  fprintf(stderr, "using move %d for enemy ant @%d,%d (score should be %g, is now %g)\n",
          worstmove, orig_pos.col, orig_pos.row, worstscore, state.evalScore);
#endif
  worstscore = state.evalScore;
  return worstscore;
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

  for(size_t i=0;i<state.myAnts.size();i++) {
    score = iterateAnt(score, state.myAnts[i]);
  }

  // evaluate enemy ant moves
  for(size_t i=0;i<state.enemyAnts.size();i++) {
    score = iterateEnemyAnt(score, state.enemyAnts[i]);
  }

  // now re-evaluate in backwards order
  for(int i=(int)state.myAnts.size()-2;i>=0;i--) {
    score = iterateAnt(score, state.myAnts[i]);
  }

  for(size_t i=0;i<state.myAnts.size();i++) {
    state.myAnts[i]->CommitMove(state);
  }

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
