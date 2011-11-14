#ifndef __SCORE_H
#define __SCORE_H

#include "Square.h"
#include "State.h"

#include <assert.h>
#include <limits.h>

//#define VERBOSE

// future-reward discount factor (usually gamma in reinforcement learning
// literature)
const float kDiscount = 0.7; // should be < 1/sqrt(2) for forward progress?
const float kFoodSpawnProb = 1.0/65536.0;
const float kHillPriority = 10.0;
const float kAntPriority = 1e-4;

static double ExploreScore(const State &state, const Square &sq) {
  int turndelta = state.turn - sq.lastSeen;
  return kFoodSpawnProb * turndelta *
    pow(kDiscount, sq.distance[Square::DIST_MY_ANTS]);
}

static double FoodScore(const State &state, const Square &sq) {
  // FIXME: if an ant goes between three pieces of food, such that going
  // towards one moves away from two others (usually one of them has to be
  // equidistant) then the ant will get stuck with this metric.  we need a way
  // out, possibly based on the time-since-observed of the food making older
  // food more urgent, or something, to break the tie and to make sitting still
  // near food always bad.
  return pow(kDiscount, sq.distance[Square::DIST_MY_ANTS] - 1);
}

static double SquareScore(const State &state, const Square &sq) {
  double score = ExploreScore(state, sq);
  if(sq.isFood)
   score += FoodScore(state, sq);
  return score;
}

static double AntScore(const State &state, const Ant *ant) {
  // contribution of score from this ant
  const Square &sq = state.grid(ant->pos_);

  // existence of this ant counts for some fixed number of points; if the ant
  // dies, we gain/lose those points
  double score = ant->team_ == 0 ? 20 : -1;

  int enemy_hill_dist = sq.distance[Square::DIST_ENEMY_HILLS];
  if(enemy_hill_dist != INT_MAX)
    score += kHillPriority*pow(kDiscount, sq.distance[Square::DIST_ENEMY_HILLS]);
  int enemy_ant_dist = sq.distance[Square::DIST_ENEMY_ANTS];
    score += kAntPriority*pow(kDiscount, sq.distance[Square::DIST_ENEMY_ANTS]);
  return score;
}

#endif // __SCORE_H
