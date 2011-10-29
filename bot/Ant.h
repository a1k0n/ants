#ifndef __ANT_H
#define __ANT_H

#include "Location.h"

struct State;
struct Ant
{
  // pos_ is the current position of the ant (implied after the move) -- to get
  // the original position, use pos_.prev(move_).
  int id_;
  Location pos_;
  bool dead_;
  int move_;

  Ant() { move_ = -1; dead_ = false; }
  Ant(int id, Location pos):id_(id), pos_(pos) { move_ = -1; dead_ = false; }

  bool Move(State &s, int move);
  void CommitMove(State &s);
};

#endif

