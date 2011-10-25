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
  int rows, cols,
      turn, turns,
      noPlayers;
  // various squared radii
  double attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;
  int viewBoxSize;
  std::vector<double> scores;
  bool gameover;

  std::vector<std::pair<Location, int> > visibilityAdjust[TDIRECTIONS];
  std::vector<std::pair<Location, int> > attackAdjust[TDIRECTIONS];

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

  int wrapRow(int r) { return (r+rows)%rows; }
  int wrapCol(int c) { return (c+cols)%cols; }
  double distance2(const Location &loc1, const Location &loc2);

  const Square& grid(const Location &l) const { return grid_[l.row][l.col]; }
  Square& grid(const Location &l) { return grid_[l.row][l.col]; }

  const Square& grid(int r, int c) const { return grid_[r][c]; }
  Square& grid(int r, int c) { return grid_[r][c]; }

  void setViewRadius(int radius2);

  void updateVisionInformation();
  void updateStateEstimate();
  void updateDistanceInformation();

 private:
  void updateAntVisibility(Location l);
  void computeCircleDelta(const Location &delta,
      std::vector<std::pair<Location, int> > *adjust);
  void bfs(std::vector<Location> seed, int type);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
