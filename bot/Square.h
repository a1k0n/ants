#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

// struct for representing a square in the grid.
struct Square
{
  bool isWater:1,
       isHill:1,
       isFood:1,
       isExplored:1;
  int ant, hillPlayer;
  int lastSeen;   // turn# that this square was last observed by an ant
  int visibility; // number of ants who can see this square
  enum {
    DIST_FOOD = 0,
    DIST_FRONTIER = 1,
    DIST_INVISIBLE = 2,
    DIST_OUR_HILL = 3,
    DIST_ENEMY_ANTS = 4,
    DIST_ENEMY_HILL = 5,
    NUM_DISTANCES = 6
  };
  int distance[NUM_DISTANCES];
  //std::vector<int> deadAnts; // we don't really need to track these do we?

  Square()
  {
    isExplored = isWater = isHill = isFood = 0;
    visibility = 0;
    ant = hillPlayer = -1;
  }

  //resets the information for the square except water information
  void reset()
  {
    visibility = 0;
    isHill = 0;
    isFood = 0;
    ant = hillPlayer = -1;
  }
};

#endif //SQUARE_H_
