#ifndef SQUARE_H_
#define SQUARE_H_

#include <vector>

// struct for representing a square in the grid.
struct Square
{
  bool isVisible, isWater, isHill, isFood, isExplored;
  int ant, hillPlayer;
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
    isVisible = isExplored = isWater = isHill = isFood = 0;
    ant = hillPlayer = -1;
  }

  //resets the information for the square except water information
  void reset()
  {
    isVisible = 0;
    isHill = 0;
    isFood = 0;
    ant = hillPlayer = -1;
  }
};

#endif //SQUARE_H_
