#include "Bot.h"

#include <limits.h>
#include <math.h>

using namespace std;

// future-reward discount factor (usually gamma in reinforcement learning
// literature)
const float kDiscount = 0.8;
const float kFoodSpawnProb = 1.0/65536.0;

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
double Bot::evaluate()
{
  double score = 0;

  // get distance from all of our ants on entire grid
  Grid<int> myAntDist;
  vector<Location> seed;
  for(int i=0;i<state.myAnts.size();i++)
    seed.push_back(state.myAnts[i].pos_);
  state.bfs(seed, myAntDist);

  double explore_score = 0;
  for(int r=0;r<state.rows;r++) {
    for(int c=0;c<state.cols;c++) {
      const Square &s = state.grid(r,c);
      if(s.isWater)
        continue;
      if(s.visibility > 0)
        continue;
      int dist = myAntDist(r,c);
      int turndelta = state.turn - s.lastSeen;
      explore_score += kFoodSpawnProb * turndelta * pow(kDiscount, dist);
    }
  }
  cerr << "explore_score: " << explore_score << endl;

  // - count up number of enemy ants; each ant is worth -1
  double enemy_score = 0;
  enemy_score -= state.enemyAnts.size();
  // - consequently, obtaining food is worth 1 -- food is thus worth
  //   max{i}(1*discount^dist_to_ant[i]); discount is 0.9 or something
  map<Location, Food>::iterator i = state.food.begin();
  double food_score = 0;
  for(;i != state.food.end();++i) {
    Food& f = i->second;
    //cerr << "food @" << i->first.col << "," << i->first.row <<
    //  " vs " << state.myAnts.size() << " ants" << endl;
    int closest_ant = -1;
    int closest_enemy = -1;
    int mydist = f.minDistance(state, state.myAnts, &closest_ant);
    int enemydist = f.minDistance(state, state.enemyAnts, &closest_enemy);
    // cerr << "mydist="<<mydist<<" enemydist="<<enemydist<<endl;
    // Note: a distance of 1 means the ant gets it
    if(enemydist < mydist)
      score -= pow(kDiscount, enemydist - 1);
    else if(mydist < enemydist) {
      double this_food_score = pow(kDiscount, mydist - 1);
      cerr << "food_score(" << i->first.col << "," << 
        i->first.row << ") = " << this_food_score << endl;
      food_score += this_food_score;
    }
    // tie destroys the food, delta score=0
  }
  cerr << "total food score: " << food_score << endl;
  // - count up hills; each is worth 2000 (losing it loses 2000 points)
  //score += 2000*state.myHills.size();
  // - smashing an enemy hill is worth 1000; if an ant moves onto one without
  //   getting killed... in other words, known existing enemy hills are worth
  //   -1000. (but the act of scouting one is positive, not negative)
  //score -= 1000*state.enemyHills.size();
  // - consider potential enemy ants in unseen locations -- an unseen enemy ant
  //   could smash our hill in N turns, so the penalty would be
  //   -2000*discount^N*p(ant@N)

  // - consider potential food in unseen locations
  //   (1*discount^dist_to_ant_xy)*p(food@xy) for each location revealed
  double reveal_score = 0;
  for(int a=0;a<state.myAnts.size();a++) {
    if(state.myAnts[a].dead_)
      continue;
    // - count up number of our live ants; each ant is worth 1
    score ++;

    int m = state.myAnts[a].move_;
    if(m == -1)
      continue; // nothing is revealed when the ant doesn't move
    Location oldloc = state.myAnts[a].pos_.prev(m);
    for(int idx=0;idx<state.visibilityAdjust[m].size();idx++) {
      const pair<Location, int> &adj = state.visibilityAdjust[m][idx];
      if(adj.second > 0) {
        Location l = state.wrapLocation(adj.first + oldloc);
        const Square &s = state.grid(l);
        // stuff that's already visible doesn't confer any advantage...
        // but we might want to provide some small bonus for redundant scouting
        if(s.visibility != 0)
          continue;
        int turndelta = state.turn - s.lastSeen;
        // probability of food here is kFoodSpawnProb*turndelta
        // value of this food depends on the closest ant's distance to this
        // square
        int ourdist = state.myAntsDist(l);
        // 7 here is the assumed radius in turns
        reveal_score += kFoodSpawnProb*turndelta*pow(kDiscount, ourdist);
      }
    }
  }
  cerr << "reveal_score: " << reveal_score << endl;
  score += reveal_score;

  // try not to block our anthills and try not to let any enemy ants near
  set<Location>::iterator hill = state.myHills.begin();
  for(; hill != state.myHills.end(); ++hill) {
    if(!state.grid(*hill).myAnts.empty())
      score -= 0.1; // decrease one point for an ant on top of an anthill
    // TODO: minus points for ants near hill,
    // minus points for not being able to see area around anthill (AKA
    // potential enemy ants)
  }
  score = explore_score + enemy_score + food_score + reveal_score;
  cerr << "enemy_score: " << enemy_score << endl;
  cerr << "total score: " << score << endl;
  return score;
}

double Bot::iterateAnt(double bestscore, Ant &a)
{
  cerr << "moving ant @"<<a.pos_.col<<", "<<a.pos_.row<<endl;
  int bestmove = -1;
  for(int m=0;m<TDIRECTIONS;m++) {
    if(!a.Move(state, m))
      continue;
    double score = evaluate();
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

  double score = evaluate();
  cerr << "eval(0) = " << score << endl;
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
