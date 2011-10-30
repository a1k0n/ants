#ifndef __SCORE_H
#define __SCORE_H

#include "Square.h"
#include "State.h"

#include <assert.h>

// future-reward discount factor (usually gamma in reinforcement learning
// literature)
const float kDiscount = 0.8;
const float kFoodSpawnProb = 1.0/65536.0;

static double ExploreScore(const State &state, const Square &sq) {
  int turndelta = state.turn - sq.lastSeen;
  return kFoodSpawnProb * turndelta * pow(kDiscount, sq.myAntDist);
}

static double FoodScore(const State &state, const Square &sq) {
  return pow(kDiscount, sq.myAntDist - 1);
}

static double SquareScore(const State &state, const Square &sq) {
  double score = ExploreScore(state, sq);
  if(sq.isFood)
   score += FoodScore(state, sq);
  return score;
}

static double AntScore(const State &state, const Ant &ant) {
  // contribution of score from this ant
  // this is slightly tricky when there are two colliding ants, since each
  // colliding ant is worth -1 but only one of the ants can be said to have
  // 'caused' the collision, so one of them is worth -2 and the other is worth 0
  size_t nAnts = state.grid(ant.pos_).myAnts.size();
  assert(nAnts > 0);
  //fprintf(stderr, "AntScore(%d,%d) nAnts=%d\n", ant.pos_.col, ant.pos_.row, nAnts);
  if(nAnts == 1) return 0;
  if(nAnts == 2) return -2;
  return -1;
}

#endif // __SCORE_H
