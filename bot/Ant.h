#ifndef __ANT_H
#define __ANT_H

#include "Location.h"

#include <set>
#include <stdio.h>

struct State;
struct Ant
{
  // pos_ is the current position of the ant (implied after the move) -- to get
  // the original position, use pos_.prev(move_).
  int team_;
  Location pos_, origPos_;
  int move_;
  int nEnemies_;
  bool dead_;
  bool combat_;
  std::set<Ant*> enemies_;

  // FIXME
  //double scoreContrib_;

  double moveScore_[5], lossScore_;

  // Dirichlet distribution of superior moves, obtained via Gibbs sampling
  int dirichlet_[5*5*5];
  // ants which we are conditionally dependent on
  Ant *dependUp_, *dependLeft_;

  Ant() { dependUp_ = dependLeft_ = NULL;  Init(); }
  Ant(int team, Location pos):team_(team), pos_(pos), origPos_(pos) {
    dependUp_ = dependLeft_ = NULL; Init();
  }

  void Init() {
    move_ = 0; nEnemies_ = 0; dead_ = combat_ = false;
    for(int j=0;j<5*5*5;j++)
      dirichlet_[j] = 1;
  }

  bool CanMove(State &s, int move);

  // evaluate territory changes
  bool TerritoryMove(State &s, int move);
  // evaluate ant combat moves
  bool CheapMove(State &s, int move);
  int GibbsStep(State &s);
  int SampleMove(State &s);

  void CommitMove(State &s);

  // populate moveScore_
  void ComputeDeltaScores(State &s);

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
