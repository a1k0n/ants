#include "Ant.h"
#include "State.h"
#include "Score.h"

#include <algorithm>
#include <assert.h>
#include <float.h>

//#define BLAH
#define CONVERGENCE_CHECK

bool Ant::TerritoryMove(State &s, int move)
{
  if(move == move_)
    return true;
  Square &oldsq = s.grid(pos_);

  Location newpos = origPos_.next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater) return false;
  if(newsq.nextAnt) return false;
  assert(oldsq.nextAnt == this);

  // TODO: resolve battles with enemy ants with this ant removed

#ifdef VERBOSE
  double oldeval = s.evalScore;
#endif
  // update the distance grid and evaluation scores
  if(team_ == 0) {
    s.updateAntPos(pos_, newpos);
#ifdef VERBOSE
    fprintf(stderr, "Ant::Move: evalScore updateAntPos(%d,%d->%d,%d) %+g=%g\n",
            origPos_.col, origPos_.row, newpos.col, newpos.row,
            s.evalScore-oldeval, scoreContrib_);
#endif
  } else {
    // FIXME: update enemy ant distances similarly
  }

  // Note: combat moves can affect multiple ants (both friendly and enemy)
  // so doCombatMove will recursively update evalScore from each ant if there's
  // a change

  oldsq.nextAnt = NULL;
  newsq.nextAnt = this;
  pos_ = newpos;
  move_ = move;

  // TODO: resolve battles with enemy ants with this ant added (maybe just by
  // updating the battle strength grid and then doing a final check during eval)

  return true;
}

bool Ant::CanMove(State &s, int move)
{
  Location newpos = origPos_.next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater || newsq.isFood) return false;
  if(newsq.nextAnt && newsq.nextAnt != this) return false;
  return true;
}

bool Ant::CombatMove(State &s, int move)
{
  if(move == move_)
    return true;
  Square &oldsq = s.grid(pos_);

  Location newpos = origPos_.next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater) return false;
  if(newsq.nextAnt) return false;
  assert(oldsq.nextAnt == this);

  // penalize standing on a hill
  // does this even belong here?
  if(oldsq.isHill && oldsq.hillPlayer == 0) s.evalScore += 1;
  if(newsq.isHill && newsq.hillPlayer == 0) s.evalScore -= 1;

  // undo the old move
  s.doCombatMove(this, move_, -1);

  oldsq.nextAnt = NULL;
  newsq.nextAnt = this;
  pos_ = newpos;
  move_ = move;

  // do the new move
  s.doCombatMove(this, move, 1);

  UpdateScore(s);

  return true;
}

bool Ant::CheapMove(State &s, int move)
{
  if(move == move_)
    return true;
  Square &oldsq = s.grid(pos_);

  Location newpos = origPos_.next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater) return false;
  if(newsq.nextAnt) return false;
  assert(oldsq.nextAnt == this);

  oldsq.nextAnt = NULL;
  newsq.nextAnt = this;
  pos_ = newpos;
  move_ = move;

  // scores aren't very meaningful things to know after we've committed our moves
  // UpdateScore(s);

  return true;
}

void Ant::MoveTowardEnemy(State &s, int dir)
{
  if(dir == 0)
    CombatMove(s, 0);
  int best_enemy_dist = INT_MAX;
  int best_move = 0;
  moving_ = true;
  for(int move=1;move<5;move++) {
    Location newpos = origPos_.next(move);
    Square &newsq = s.grid(newpos);
    if(newsq.isWater || newsq.isFood) continue;
    if(newsq.nextAnt && newsq.nextAnt != this) {
      // attempt to recursively move ants out of the way
      if(!newsq.nextAnt->moving_ && newsq.nextAnt->team_ == team_)
        newsq.nextAnt->MoveTowardEnemy(s, dir);
      if(newsq.nextAnt && newsq.nextAnt != this) continue; // they're still in the way; give up
    }
    int dist = dir * newsq.distance[team_ == 0 ? Square::DIST_ENEMY_ANTS : Square::DIST_MY_ANTS];
    if(dist < best_enemy_dist) {
      best_enemy_dist = dist;
      best_move = move;
    }
  }
  CombatMove(s, best_move);
  moving_ = false;
}

static bool coinflip(double c)
{
  return drand48() < c;
}

void Ant::MaximizeMove(State &s)
{
#ifdef MULTIPASS
  if(committed_)
    return;
#endif
  int dir_base = 0;
#ifdef MINIMAX_VALUE
  double bestvalue = -DBL_MAX;
#else
  int bestvalue = INT_MIN;
#endif
  int bestmove = 0, nbest = 0;
#ifdef CONDITIONAL
  if(dependUp_) {
#ifdef MULTIPASS
    dependUp_->CommitMove(s);
#endif
    dir_base += 5*dependUp_->move_;
  }
  if(dependLeft_) {
#ifdef MULTIPASS
    dependLeft_->CommitMove(s);
#endif
    dir_base += 25*dependLeft_->move_;
  }
#endif
#ifdef VERBOSE0
  fprintf(stderr, "ant %3d,%3d p%d: ",
          origPos_.col, origPos_.row, team_);
#ifdef CONDITIONAL
  fprintf(stderr, "maximizing dirichlet(dep=%c,%c conv=%2d)=[",
          CDIRECTIONS[(dir_base/5)%5],
          CDIRECTIONS[(dir_base/25)%5],
          converged_[dir_base/5]);
#else
  fprintf(stderr, "maximizing dirichlet(conv=%2d)=[",
          converged_[dir_base/5]);
#endif
#endif
  for(int d=0;d<5;d++) {
    if(!CanMove(s, d))
      continue;
#ifdef MINIMAX_VALUE
    // choose the move with the best minimum value
    double value = minvalue_[dir_base+d];
#else
    int value = dirichlet_[dir_base+d];
#endif
    if(value > bestvalue) {
      bestvalue = value;
      bestmove = d;
      nbest = 1;
    } else if(value == bestvalue) {
      nbest++;
      if(coinflip(1.0/nbest))
        bestmove = d;
    }
#ifdef VERBOSE0
    fprintf(stderr, "%c:%g ", CDIRECTIONS[d], (double)value);
#endif
  }
#ifdef VERBOSE0
  fprintf(stderr, "\b] ");
#endif
  CheapMove(s, bestmove);
#ifdef VERBOSE0
  double value = s.evalScore + (dead_ ? 0 : moveScore_[bestmove]);
  fprintf(stderr, "(ant:%g + territory:%g)=%g %c\n", scoreContrib_,
          moveScore_[bestmove], value, CDIRECTIONS[bestmove]);
#endif
}

void Ant::CommitMove(State &s)
{
  committed_ = true;
  s.CommitMove(origPos_, move_);
}

void Ant::ComputeDeltaScores(State &s)
{
  assert(pos_ == origPos_); // can't have moved yet to compute this
  // Tweak: since these scores are based solely on exploration and food
  // collection, penalize standing still as if it keeps you two turns further
  // away
  double baseScore = s.evalScore * kFoodDiscount*kFoodDiscount;
  for(int i=4;i>=0;i--) { // 0 has to come last
    TerritoryMove(s, i);
    moveScore_[i] = s.evalScore - baseScore;
#ifdef BLAH
    fprintf(stderr, "ant (%d,%d) move %c=%g\n",
            origPos_.col, origPos_.row,
            CDIRECTIONS[i], moveScore_[i]);
#endif
  }
  moveScore_[0] = baseScore;
}

int Ant::SampleMove(State &s)
{
  int dir, sum=0, fullsum=0;
  bool can_move[5];
  int dir_base = 0;
#ifdef CONDITIONAL
  if(dependUp_)
    dir_base += dependUp_->move_;
  if(dependLeft_)
    dir_base += 5*dependLeft_->move_;
#endif
  if(converged_[dir_base] != -1) {
    dir = converged_[dir_base];
    // hopefully this is the common case
    if(CombatMove(s, dir))
      return dir;
  }
  dir_base *= 5;
  for(dir = 0;dir<5; dir++) {
    // check whether another ant is in the way
    fullsum += dirichlet_[dir_base+dir];
    can_move[dir] = CanMove(s, dir);
    if(can_move[dir])
      sum += dirichlet_[dir_base+dir];
  }
  int idx = lrand48() % sum;
#ifdef BLAH
  fprintf(stderr, "idx=%d, ", idx);
#endif
  // this is ugly as hell but works
  for(dir = 0; !can_move[dir] || idx >= dirichlet_[dir_base+dir]; dir++) {
    if(can_move[dir])
      idx -= dirichlet_[dir_base+dir];
  }
  // if over 93% of the probability mass is in this direction, we can probably
  // stop sampling here.
#ifdef CONVERGENCE_CHECK
  if(dirichlet_[dir_base+dir] > 15*fullsum/16) {
    // 15/16 = ~93%
    converged_[dir_base/5] = dir;
  }
#endif
#ifdef BLAH
  fprintf(stderr, "move=%d\n", dir);
#endif
  CombatMove(s, dir);
  return dir;
}

void Ant::UpdateDirichlet(State &s)
{
  bool minimize = team_ == 0 ? false : true;
  double bestvalue = -DBL_MAX;
  int bestmoves[5], nbest = 0, ndifferent = 0;
  double lastvalue = bestvalue;

  int dir_base = 0;
#ifdef CONDITIONAL
  if(dependUp_)
    dir_base += dependUp_->move_;
  if(dependLeft_)
    dir_base += 5*dependLeft_->move_;
#endif
  if(converged_[dir_base] != -1) {
    int move = converged_[dir_base];
    // hopefully this is the common case
    if(CanMove(s, move))
      return;
  }
  dir_base *= 5;

  for(int move=0;move<5;move++) {
    if(CombatMove(s, move)) {
      double value = s.evalScore + (dead_ ? 0 : moveScore_[move]);
      if(minimize) value = -value;
#ifdef MINIMAX_VALUE
      minvalue_[dir_base+move] = std::min(value, minvalue_[dir_base+move]);
#ifdef BLAH
      fprintf(stderr, "ant (%d,%d) %c=(%g%+g)=%g min=%g; ant=%g best=%g\n",
              origPos_.col, origPos_.row, CDIRECTIONS[move],
              s.evalScore, moveScore_[move], value, minvalue_[dir_base+move],
              scoreContrib_, bestvalue);
      value = minvalue_[dir_base+move];
#endif
#else
#ifdef BLAH
      fprintf(stderr, "ant (%d,%d) %c=(%g%+g)=%g; ant=%g\n",
              origPos_.col, origPos_.row, CDIRECTIONS[move],
              s.evalScore, moveScore_[move], value, scoreContrib_);
#endif
#endif
      if(value != lastvalue)
        ndifferent++;
      if(value > bestvalue) {
        nbest = 0;
        bestvalue = value;
        bestmoves[nbest++] = move;
      } else if(value == bestvalue) {
        bestmoves[nbest++] = move;
      }
    }
  }

  // update dirichlet distribution
  // there should be more than 1 unique score for us to bother updating
  if(ndifferent > 1) {
    for(int d=0;d<nbest;d++)
      dirichlet_[dir_base+bestmoves[d]] += kDirichletIncrement;
  }
#ifdef BLAH
  fprintf(stderr, "ant (%d,%d) [nd=%d nb=%d b0=%c] dirichlet(dep=%c,%c)=[",
          origPos_.col, origPos_.row, ndifferent, nbest, CDIRECTIONS[bestmoves[0]],
          CDIRECTIONS[(dir_base/5)%5],
          CDIRECTIONS[(dir_base/25)%5]);
  for(int d=0;d<5;d++) {
    if(!CanMove(s, d)) continue;
    fprintf(stderr, "%c:%d ", CDIRECTIONS[d], dirichlet_[dir_base+d]);
  }
  fprintf(stderr, "\b] ");
#endif
}

void Ant::UpdateScore(State &s)
{
  s.evalScore -= scoreContrib_;
  scoreContrib_ = AntScore(s, this);
  s.evalScore += scoreContrib_;
}
