#ifndef __ANT_H
#define __ANT_H

#include "Location.h"

#include <vector>
#include <stdio.h>

static const int kDirichletAlpha = 1;

// use conditional probabilities when sampling (seems to be weaker due to
// limited sampling)
#undef CONDITIONAL

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
  bool committed_;
  std::vector<Ant*> enemies_;

  double scoreContrib_;

  double moveScore_[5];

  // Dirichlet distribution of superior moves, obtained via Gibbs sampling
#ifdef CONDITIONAL
  int dirichlet_[5*5*5];
  int converged_[5*5];
  // ants which we are conditionally dependent on
  Ant *dependUp_, *dependLeft_;
#else
  int dirichlet_[5];
  int converged_[1];
#endif

  Ant() { Init(); }
  Ant(int team, Location pos):team_(team), pos_(pos), origPos_(pos) {
    Init();
  }

  void Init() {
#ifdef CONDITIONAL
    dependUp_ = dependLeft_ = NULL;
#endif
    scoreContrib_ = 0;
    move_ = 0; nEnemies_ = 0; dead_ = committed_ = false;
#ifdef CONDITIONAL
    for(int j=0;j<5*5*5;j++)
      dirichlet_[j] = kDirichletAlpha;
    for(int j=0;j<5*5;j++)
      converged_[j] = -1;
#else
    for(int j=0;j<5;j++)
      dirichlet_[j] = 1;
    converged_[0] = -1;
#endif
  }

  bool CanMove(State &s, int move);

  // evaluate territory changes
  bool TerritoryMove(State &s, int move);
  // evaluate ant combat moves
  bool CombatMove(State &s, int move);
  // just mark the ant position in the grid
  bool CheapMove(State &s, int move);
  int GibbsStep(State &s);
  int SampleMove(State &s);

  void MaximizeMove(State &s);
  void CommitMove(State &s);

  // populate moveScore_
  void ComputeDeltaScores(State &s);

  void UpdateScore(State &s);

  void CheckCombatDeath() {
    for(size_t i=0; i < enemies_.size(); ++i) {
      if(enemies_[i]->nEnemies_ <= nEnemies_) {
        dead_ = true;
        return;
      }
    }
    dead_ = false;
  }

  void dumpEnemies() {
    if(enemies_.empty()) return;
    fprintf(stderr, "enemies: ");
    for(size_t i=0; i < enemies_.size(); ++i) {
      Ant *e = enemies_[i];
      Location l = e->origPos_;
      fprintf(stderr, "(%d,%d) ", l.col, l.row);
    }
    fprintf(stderr, "\n");
  }
};

#endif
