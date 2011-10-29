#include "Food.h"
#include "Grid.h"
#include "Square.h"
#include "State.h"

#include <vector>

using namespace std;

int Food::minDistance(const State &state,
                      const vector<Ant> &ants,
                      int *closest_idx)
{
  if(lastUpdate_ != state.turn)
    UpdateGrid(state);

  int mindist = INT_MAX;
  for(size_t i = 0; i < ants.size(); i++) {
    if(ants[i].dead_)
      continue;
    int dist = distance_(ants[i].pos_);
    if(dist < mindist) {
      mindist = dist;
      *closest_idx = i;
    }
  }

  return mindist;
}

void Food::UpdateGrid(const State &state)
{
  std::vector<Location> seed(1, pos_);
  state.bfs(seed, distance_);

//  cerr << "distance info for food @"<<pos_.col<<","<<pos_.row<<endl;
//  cerr << distance_;

  lastUpdate_ = state.turn;
}

