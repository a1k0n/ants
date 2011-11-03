#ifndef __ANT_H
#define __ANT_H

#include "Location.h"

#include <set>

struct State;
struct Ant
{
  // pos_ is the current position of the ant (implied after the move) -- to get
  // the original position, use pos_.prev(move_).
  int team_;
  Location pos_;
  int move_;
  int nEnemies_;
  std::set<Ant*> enemies_;

  Ant() { Init(); }
  Ant(int team, Location pos):team_(team), pos_(pos) { Init(); }

  void Init() { move_ = -1; }

  bool Move(State &s, int move);
  void CommitMove(State &s);
};

#endif

