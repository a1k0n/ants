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
  if(newsq.isWater) return false;
  if(newsq.nextAnt && newsq.nextAnt != this) return false;
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

static bool coinflip(double c)
{
  return drand48() < c;
}

void Ant::CommitMove(State &s)
{
  if(committed_)
    return;
  int dir_base = 0;
  double bestvalue = -DBL_MAX;
  int bestmove = 0, nbest = 0;
  if(dependUp_) {
    dependUp_->CommitMove(s);
    dir_base += 5*dependUp_->move_;
  }
  if(dependLeft_) {
    dependLeft_->CommitMove(s);
    dir_base += 25*dependLeft_->move_;
  }
  fprintf(stderr, "ant (%d,%d) (team %d): ",
          origPos_.col, origPos_.row, team_);
  fprintf(stderr, "maximizing dirichlet(dep=%c,%c, conv=%d)=[",
          CDIRECTIONS[(dir_base/5)%5],
          CDIRECTIONS[(dir_base/25)%5],
          converged_[dir_base/5]);
  for(int d=0;d<5;d++) {
    if(!CanMove(s, d))
      continue;
    if(!nsamples_[dir_base+d])
      continue;
#if 1
    double value = dirichlet_[dir_base+d];
#else // this is a terrible idea
    double value = rewardsum_[dir_base+d]/nsamples_[dir_base+d];
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
    fprintf(stderr, "%c:%g(%d) ", CDIRECTIONS[d],
            value, nsamples_[dir_base+d]);
  }
  fprintf(stderr, "\b] ");
  CheapMove(s, bestmove);
  double value = s.evalScore + (dead_ ? 0 : moveScore_[bestmove]);
  fprintf(stderr, "%c=(ant:%g + territory:%g)=%g ", CDIRECTIONS[bestmove],
          scoreContrib_, moveScore_[bestmove], value);
  fprintf(stderr, "moving %c\n", CDIRECTIONS[bestmove]);

  committed_ = true;
  s.CommitMove(origPos_, move_);
}

void Ant::ComputeDeltaScores(State &s)
{
  assert(pos_ == origPos_); // can't have moved yet to compute this
  double baseScore = s.evalScore;
  for(int i=4;i>=0;i--) { // 0 has to come last
    TerritoryMove(s, i);
    moveScore_[i] = s.evalScore - baseScore;
#ifdef BLAH
    fprintf(stderr, "ant (%d,%d) move %c=%g\n",
            origPos_.col, origPos_.row,
            CDIRECTIONS[i], moveScore_[i]);
#endif
  }
}

int Ant::SampleMove(State &s)
{
  int dir, sum=0, fullsum=0;
  bool can_move[5];
  int dir_base = 0;
  if(dependUp_)
    dir_base += dependUp_->move_;
  if(dependLeft_)
    dir_base += 5*dependLeft_->move_;
  if(converged_[dir_base] != -1) {
    dir = converged_[dir_base];
    // hopefully this is the common case
    if(CheapMove(s, dir))
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
  CheapMove(s, dir);
  return dir;
}

int Ant::GibbsStep(State &s)
{
  bool minimize = team_ == 0 ? false : true;
  double bestvalue = minimize ? DBL_MAX : -DBL_MAX;
  int bestmoves[5], nbest = 0, ndifferent = 0;
  double lastvalue = bestvalue;

  int dir_base = 0;
  if(dependUp_)
    dir_base += dependUp_->move_;
  if(dependLeft_)
    dir_base += 5*dependLeft_->move_;
  if(converged_[dir_base] != -1) {
    int move = converged_[dir_base];
    // hopefully this is the common case
    if(CheapMove(s, move))
      return move;
  }
  dir_base *= 5;

  for(int move=0;move<5;move++) {
    if(CheapMove(s, move)) {
      double value = s.evalScore + (dead_ ? 0 : moveScore_[move]);
#ifdef BLAH
      fprintf(stderr, "ant (%d,%d) %c=(%g%+g)=%g; ant=%g\n",
              origPos_.col, origPos_.row, CDIRECTIONS[move],
              s.evalScore, moveScore_[move], value, scoreContrib_);
#endif
      rewardsum_[dir_base+move] += value;
      nsamples_[dir_base+move]++;

      if(value != lastvalue)
        ndifferent++;
      if((minimize && value < bestvalue) ||
         (!minimize && value > bestvalue)) {
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
      dirichlet_[dir_base+bestmoves[d]]++;
  }
#ifdef BLAH
  fprintf(stderr, "ant (%d,%d) dirichlet(dep=%c,%c)=[",
          origPos_.col, origPos_.row,
          CDIRECTIONS[(dir_base/5)%5],
          CDIRECTIONS[(dir_base/25)%5]);
  for(int d=0;d<5;d++) {
    if(!CanMove(s, d)) continue;
    fprintf(stderr, "%c:%d ", CDIRECTIONS[d], dirichlet_[dir_base+d]);
  }
  fprintf(stderr, "\b] ");
#endif

  int move = SampleMove(s);
#ifdef BLAH
  fprintf(stderr, "ant (%d,%d) sampled_dir=%c value=%g (ant=%g)\n",
          origPos_.col, origPos_.row,
          CDIRECTIONS[move], s.evalScore, scoreContrib_);
#endif
  return move;
}

void Ant::UpdateScore(State &s)
{
  s.evalScore -= scoreContrib_;
  scoreContrib_ = AntScore(s, this);
  s.evalScore += scoreContrib_;
}
