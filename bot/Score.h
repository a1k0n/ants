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
const float kHillOffensePriority = 10.0;
const float kHillDefensePriority = 50.0;
const float kEnemyPriority = 1e-4;
const float kTieBreaker = 1e-6;
const float kMyAntValue = 2.0;
const float kEnemyAntValue = 1.0;

static double ExploreScore(const State &state, const Square &sq) {
  int turndelta = state.turn - sq.lastSeen;
  return kFoodSpawnProb * turndelta *
    pow(kDiscount, sq.distance[Square::DIST_MY_ANTS]);
 // + kTieBreaker * pow(kDiscount, sq.distance[Square::DIST_FRONTIER]);
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
  if(ant->dead_)
    return 0;

  const Square &sq = state.grid(ant->pos_);

  // existence of this ant counts for some fixed number of points; if the ant
  // dies, we gain/lose those points
  if(ant->team_ == 0) {
    double score = kMyAntValue;

    int enemy_hill_dist = sq.distance[Square::DIST_ENEMY_HILLS];
    if(enemy_hill_dist != INT_MAX)
      score += kHillOffensePriority*pow(kDiscount, enemy_hill_dist);
    int enemy_ant_dist = sq.distance[Square::DIST_ENEMY_ANTS];
    score += kEnemyPriority*pow(kDiscount, enemy_ant_dist);
    return score;
  } else {
    // enemy ants near our hill == bad
    double score = -kEnemyAntValue;
    int my_hill_dist = sq.distance[Square::DIST_MY_HILLS];
    if(my_hill_dist != INT_MAX)
      score -= kHillDefensePriority*pow(kDiscount, my_hill_dist);
    return score;
  }
}

#endif // __SCORE_H
