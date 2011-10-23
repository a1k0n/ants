#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <queue>
#include <stack>

#include "Timer.h"
#include "Square.h"
#include "Location.h"

// struct to store current state information
struct State
{
  // Variables
  int rows, cols,
      turn, turns,
      noPlayers;
  double attackradius, spawnradius, viewradius;
  double loadtime, turntime;
  std::vector<double> scores;
  bool gameover;

  std::vector<std::vector<Square> > grid_;
  std::vector<Location> myAnts, enemyAnts, myHills;
  std::set<Location> enemyHills, food;

  Timer timer;

  // Functions
  State();
  ~State();

  void setup();
  void reset();

  void makeMove(const Location &loc, int direction);

  double distance(const Location &loc1, const Location &loc2);

  const Square& grid(const Location &l) const { return grid_[l.row][l.col]; }
  Square& grid(const Location &l) { return grid_[l.row][l.col]; }

  const Square& grid(int r, int c) const { return grid_[r][c]; }
  Square& grid(int r, int c) { return grid_[r][c]; }

  void updateVisionInformation();
  void updateStateEstimate();
  void updateDistanceInformation();

 private:
  void bfs(std::vector<Location> seed, int type);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
