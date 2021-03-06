#ifndef STATE_H_
#define STATE_H_

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <map>

#include "Ant.h"
#include "Food.h"
#include "Grid.h"
#include "Location.h"
#include "Square.h"

// struct to store current state information
struct State
{
  int rows, cols,
      turn, turns,
      noPlayers;
  // various squared radii
  double attackradius2, spawnradius2, viewradius2;
  double loadtime, turntime;
  int viewBoxSize, attackBoxSize;
  std::vector<double> scores;
  bool gameover;

  double evalScore;

  std::vector<std::pair<Location, int> > visibilityAdjust[TDIRECTIONS];
  std::vector<std::pair<Location, int> > attackAdjust[TDIRECTIONS];

  Grid<Square> grid;
  std::vector<Ant*> myAnts, enemyAnts;
  std::set<Location> myHills, enemyHills;
  std::map<Location, Food> food;
  std::vector<Ant*> antsInCombat;

  int nMyAntsKilled, nEnemyAntsKilled;

  // Functions
  State();
  ~State();

  void setup();
  void reset();

  void CommitMove(const Location &loc, int direction);

  int wrapRow(int r) { return (r+rows)%rows; }
  int wrapCol(int c) { return (c+cols)%cols; }
  Location wrapLocation(const Location &l) {
    return Location(wrapRow(l.row), wrapCol(l.col)); }
  double distance2(const Location &loc1, const Location &loc2);

  void setViewRadius(int radius2);
  void setAttackRadius(int radius2);

  void updateVisionInformation();
  void updateStateEstimate();
  void updateDistanceInformation();

  // sort of a reverse-and-forward-dijkstra step
  void updateAntPos(const Location &oldpos, const Location &newpos);

  void doCombatMove(Ant *a, int move, int direction);

  void dumpDistances(int type);
 private:
  void bfs(std::vector<Location> seed, int type);
  void updateAntVisibility(Ant *a);
  void updateAntAttack(Ant *a);
  void computeCircleDelta(const Location &delta,
      std::vector<std::pair<Location, int> > *adjust,
      int viewBoxSize, int viewradius2);
};

std::ostream& operator<<(std::ostream &os, const State &state);
std::istream& operator>>(std::istream &is, State &state);

#endif //STATE_H_
