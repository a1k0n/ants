#include "Ant.h"
#include "State.h"
#include "Score.h"

#include <algorithm>
#include <assert.h>

bool Ant::Move(State &s, int move)
{
  if(move == move_)
    return true;
  Square &oldsq = s.grid(pos_);

  Location newpos = pos_.prev(move_).next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater) return false;
  if(newsq.nextAnt) return false;
  assert(oldsq.nextAnt == this);

  double antscore = AntScore(s, this);
  s.evalScore -= antscore;
#ifdef VERBOSE
  fprintf(stderr, "Ant::Move: evalScore -AntScore %g\n", antscore);
#endif

  // penalize standing on a hill
  if(oldsq.isHill && oldsq.hillPlayer == 0) s.evalScore += 10;
  if(newsq.isHill && newsq.hillPlayer == 0) s.evalScore -= 10;

  // TODO: resolve battles with enemy ants with this ant removed

  double oldeval = s.evalScore;
  // update the distance grid and evaluation scores
  s.updateAntPos(pos_, newpos);
#ifdef VERBOSE
  fprintf(stderr, "Ant::Move: evalScore updateAntPos(%d,%d->%d,%d) %+g\n",
          pos_.col, pos_.row, newpos.col, newpos.row,
          s.evalScore-oldeval);
#endif

  oldsq.nextAnt = NULL;
  newsq.nextAnt = this;
  pos_ = newpos;
  move_ = move;

  // TODO: resolve battles with enemy ants with this ant added (maybe just by
  // updating the battle strength grid and then doing a final check during eval)

  antscore = AntScore(s, this);
  s.evalScore += antscore;
#ifdef VERBOSE
  fprintf(stderr, "Ant::Move: evalScore +AntScore %g\n", antscore);
#endif

  return true;
}

void Ant::CommitMove(State &s)
{
  if(move_ != -1)
    s.CommitMove(pos_.prev(move_), move_);
}
