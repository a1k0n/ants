#include "Bot.h"
#include <limits.h>

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

double Bot::evaluate()
{
  double score = 0;
  // - count up number of our ants; each ant is worth 1
  score += state.myAnts.size();
  // - count up number of enemy ants; each ant is worth -1
  score -= state.enemyAnts.size();
  // - consequently, obtaining food is worth 1 -- food is thus worth
  //   max{i}(1*discount^dist_to_ant[i]); discount is 0.9 or something
  // - count up hills; each is worth 2000 (losing it loses 2000 points)
  score += 2000*state.myHills.size();
  // - smashing an enemy hill is worth 1000; if an ant moves onto one without
  //   getting killed... in other words, known existing enemy hills are worth
  //   -1000. (but the act of scouting one is positive, not negative)
  score -= 1000*state.enemyHills.size();
  // - consider potential enemy ants in unseen locations -- an unseen enemy ant
  //   could smash our hill in N turns, so the penalty would be
  //   -2000*discount^N*p(ant@N)
  // - consider potential food in unseen locations
  //   (1*discount^dist_to_ant_xy)*p(food@xy) for each location revealed
  return score;
}

static bool coinflip(float threshold)
{
  return drand48() <= threshold;
}

int Bot::moveAnt(Location loc)
{
  // First, pick a best move for this ant
  int bestdir, bestcost;
  movingAnts.insert(loc);
re_pick:
  bestdir = -1;
  bestcost = INT_MAX;
  for(int dir=0;dir<TDIRECTIONS;dir++) {
    Location nextloc = loc.next(dir);
    const Square &s = state.grid(nextloc);
    // can't move onto water
    if(s.isWater) continue;
    // can't move onto a square where another ant has already moved to
    if(s.ant == 0 &&
       movedAnts.find(nextloc) != movedAnts.end())
      continue;
    int food_dist = s.distance[Square::DIST_FOOD];
    int hive_dist = s.distance[Square::DIST_ENEMY_HILL];
    int frontier_dist = s.distance[Square::DIST_FRONTIER];
    int invisible_dist = s.distance[Square::DIST_INVISIBLE];
    // only go for the frontier if we can't see any food
    // this is backwards though.. we need to assign food to ants, not vice versa
    if(food_dist == INT_MAX)
      food_dist = frontier_dist;
    if(food_dist == INT_MAX)
      food_dist = invisible_dist;
    int cost = std::min(hive_dist, food_dist);
    if(cost < bestcost || (cost == bestcost && coinflip(0.33))) {
      bestcost = cost;
      bestdir = dir;
    }
  }
  // Now, try to make the move.  If there's another ant in the way, try to move
  // that one first, and then pick a new move for us.
  if(bestdir == -1) {
    // we have nowhere to move, apparently
    // lock it in that we're stuck
    movedAnts.insert(loc);
    unmovedAnts.erase(loc);
    movingAnts.erase(loc);
    return -1;
  }
  Location bestloc = loc.next(bestdir);
  if(state.grid(bestloc).ant == 0 &&
     movedAnts.find(bestloc) == movedAnts.end()) {
    // there's an unmoved ant in our way; move him first
    if(movingAnts.find(bestloc) == movingAnts.end())
      moveAnt(bestloc);
    if(state.grid(bestloc).ant == 0 &&
       movedAnts.find(bestloc) != movedAnts.end()) {
      // argh, some other ant moved into the way.  we need to pick a new move.
      goto re_pick;
    }
  }
  movedAnts.insert(bestloc);
  unmovedAnts.erase(loc);
  state.makeMove(loc, bestdir);
  movingAnts.erase(loc);
  return bestdir;
}

//makes the bots moves for the turn
void Bot::makeMoves()
{
  cerr << "turn " << state.turn << ":" << endl;
  cerr << state << endl;

  movedAnts.clear();
  unmovedAnts.clear();
  movingAnts.clear();
  //picks out moves for each ant
  for(int ant=0; ant<(int)state.myAnts.size(); ant++)
    unmovedAnts.insert(state.myAnts[ant]);

  while(!unmovedAnts.empty()) {
    moveAnt(*(unmovedAnts.begin()));
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
