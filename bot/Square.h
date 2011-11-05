#ifndef SQUARE_H_
#define SQUARE_H_

#include "Ant.h"
#include "Location.h"

#include <vector>

// struct for representing a square in the grid.
struct Square
{
  bool isWater:1,
       isHill:1,
       isFood:1,
       isExplored:1;
  Ant *ant;
  Ant *nextAnt; // ant on this square in the next frame
  int hillPlayer;
  int lastSeen;   // turn# that this square was last observed by an ant
  int visibility; // number of ants who can see this square

  enum {
    DIST_MY_ANTS = 0,
    DIST_ENEMY_ANTS,
    DIST_ENEMY_HILLS,
    NUM_DISTANCES
  };
  int distance[NUM_DISTANCES];

  Square()
  {
    isExplored = isWater = isHill = isFood = 0;
    lastSeen = 0;
    visibility = 0;
    hillPlayer = -1;
    ant = nextAnt = NULL;
  }

  //resets the information for the square except water information
  void reset()
  {
    visibility = 0;
    isHill = 0;
    isFood = 0;
    hillPlayer = -1;
    if(ant)
      delete ant;
    nextAnt = ant = NULL;
  }
};

#endif //SQUARE_H_
