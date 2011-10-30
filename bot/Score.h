#ifndef __SCORE_H
#define __SCORE_H

#include "Square.h"
#include "State.h"

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

#endif // __SCORE_H
