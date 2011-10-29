#ifndef __FOOD_H
#define __FOOD_H

#include "Ant.h"
#include "Grid.h"

#include <vector>

struct State;
struct Food
{
  Location pos_;
  bool eaten_;
  Grid<int> distance_;
  int lastUpdate_;
  Food() { eaten_ = false; lastUpdate_ = -1; }
  Food(Location pos):pos_(pos) { eaten_ = false; lastUpdate_ = -1; }

  // deprecated from here on
  int minDistance(const State &state,
                  const std::vector<Ant> &ants,
                  int *closest_idx);
 private:
  void UpdateGrid(const State &state);
};

#endif

