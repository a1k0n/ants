#ifndef __ANT_H
#define __ANT_H

#include "Location.h"

#include <vector>
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
  bool committed_;
  std::vector<Ant*> enemies_;

  double scoreContrib_;

  double moveScore_[5];

  // Dirichlet distribution of superior moves, obtained via Gibbs sampling
  int dirichlet_[5*5*5];
  double rewardsum_[5*5*5];
  int converged_[5*5*5], nsamples_[5*5*5];
  // ants which we are conditionally dependent on
  Ant *dependUp_, *dependLeft_;

  Ant() { dependUp_ = dependLeft_ = NULL;  Init(); }
  Ant(int team, Location pos):team_(team), pos_(pos), origPos_(pos) {
    dependUp_ = dependLeft_ = NULL; Init();
  }

  void Init() {
    scoreContrib_ = 0;
    move_ = 0; nEnemies_ = 0; dead_ = committed_ = false;
    for(int j=0;j<5*5*5;j++) {
      dirichlet_[j] = 1;
      rewardsum_[j] = 0;
      nsamples_[j] = 0;
    }
    for(int j=0;j<5*5;j++) {
      converged_[j] = -1;
    }
  }

  bool CanMove(State &s, int move);

  // evaluate territory changes
  bool TerritoryMove(State &s, int move);
  // evaluate ant combat moves
  bool CheapMove(State &s, int move);
  int GibbsStep(State &s);
  int SampleMove(State &s);

  void MaximizeMove(State &s);
  void CommitMove(State &s);

  // populate moveScore_
  void ComputeDeltaScores(State &s);

  void UpdateScore(State &s);

  bool CheckCombatDeath() {
    for(size_t i=0; i < enemies_.size(); ++i) {
      if(enemies_[i]->nEnemies_ <= nEnemies_)
        return true;
    }
    return false;
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
