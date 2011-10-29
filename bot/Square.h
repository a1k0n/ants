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
  std::vector<int> myAnts, enemyAnts;
  int ant;
  int hillPlayer;
  int lastSeen;   // turn# that this square was last observed by an ant
  int visibility; // number of ants who can see this square

  Square()
  {
    isExplored = isWater = isHill = isFood = 0;
    lastSeen = 0;
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
