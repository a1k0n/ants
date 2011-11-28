#ifndef __SCORE_H
#define __SCORE_H

#include "Square.h"
#include "State.h"

#include <assert.h>
#include <limits.h>

//#define VERBOSE

// future-reward discount factor (usually gamma in reinforcement learning
// literature)
const float kDiscount = 0.9; // should be < 1/sqrt(2) for forward progress?
const float kFoodDiscount = 0.7; // should be < 1/sqrt(2) for forward progress?
const float kFoodSpawnProb = 5.0/65536.0;
const float kHillOffensePriority = 5.0;
const float kHillDiscount = 0.90;
const float kExploreDiscount = 0.70;
const float kHillDefensePriority = 10.0; //0.0;
const float kEnemyPriority = 1e-2;
const float kTieBreaker = 1e-3;
const float kMyAntValue = 5.0;
const float kEnemyAntValue = 1.0;

static inline double ExploreScore(const State &state, const Square &sq) {
  double score = 0;
  int turndelta = state.turn - sq.lastSeen;
  score += kFoodSpawnProb * turndelta *
    pow(kExploreDiscount, sq.distance[Square::DIST_MY_ANTS]);
#if 1
  if(sq.isHill && sq.hillPlayer == 0) {
    int enemy_dist = sq.distance[Square::DIST_ENEMY_ANTS];
    int my_dist = sq.distance[Square::DIST_MY_ANTS];
    if(enemy_dist != INT_MAX && my_dist != INT_MAX) {
      int dist_diff = std::min(1, std::max(-100, enemy_dist - my_dist));
      score -= pow(kDiscount, dist_diff);
    }
    if(sq.visibility == 0)
      score --;
  }
#endif
  return score;
}

static inline double FoodScore(const State &state, const Square &sq) {
  // FIXME: if an ant goes between three pieces of food, such that going
  // towards one moves away from two others (usually one of them has to be
  // equidistant) then the ant will get stuck with this metric.  we need a way
  // out, possibly based on the time-since-observed of the food making older
  // food more urgent, or something, to break the tie and to make sitting still
  // near food always bad.
  return pow(kFoodDiscount, sq.distance[Square::DIST_MY_ANTS] - 1);
}

static inline double SquareScore(const State &state, const Square &sq) {
  double score = ExploreScore(state, sq);
  if(sq.isFood)
   score += FoodScore(state, sq);
  return score;
}

static inline double AntScore(const State &state, const Ant *ant) {
  // contribution of score from this ant
  if(ant->dead_)
    return 0;

  const Square &sq = state.grid(ant->pos_);

  // existence of this ant counts for some fixed number of points; if the ant
  // dies, we gain/lose those points
  if(ant->team_ == 0) {
    double score = kMyAntValue;

    double tiebreaker = kTieBreaker * pow(0.99, sq.distance[Square::DIST_FRONTIER]);
    score += tiebreaker;

    int enemy_hill_dist = sq.distance[Square::DIST_ENEMY_HILLS];
    if(enemy_hill_dist != INT_MAX)
      score += kHillOffensePriority*pow(kHillDiscount, enemy_hill_dist);
    int enemy_ant_dist = sq.distance[Square::DIST_ENEMY_ANTS];
    if(enemy_ant_dist != INT_MAX)
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
