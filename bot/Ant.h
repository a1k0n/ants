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
  int move_;

  Ant() { Init(); }
  Ant(int id, Location pos):id_(id), pos_(pos) { Init(); }

  void Init() { move_ = -1; }

  bool Move(State &s, int move);
  void CommitMove(State &s);
};

#endif

