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
  bool dead_;
  std::set<Ant*> enemies_;

  Ant() { Init(); }
  Ant(int team, Location pos):team_(team), pos_(pos) { Init(); }

  void Init() { move_ = -1; nEnemies_ = 0; dead_ = false; }

  bool Move(State &s, int move);
  void CommitMove(State &s);

  bool CheckCombatDeath() {
    double damage = 0;
    for(std::set<Ant*>::iterator i=enemies_.begin(); i != enemies_.end(); ++i) {
      damage += 1.0/(*i)->nEnemies_;
      if(damage >= 1)
        return true;
    }
    return false;
  }

  void dumpEnemies() {
    if(enemies_.empty()) return;
    std::set<Ant*>::iterator i = enemies_.begin();
    fprintf(stderr, "enemies: ");
    for(; i != enemies_.end(); ++i) {
      Ant *e = *i;
      Location l = e->pos_.prev(e->move_);
      fprintf(stderr, "(%d,%d) ", l.col, l.row);
    }
    fprintf(stderr, "\n");
  }
};

#endif
