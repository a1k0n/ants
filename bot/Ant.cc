#include "Ant.h"
#include "State.h"

bool Ant::Move(State &s, int move)
{
  if(move == move_)
    return true;
  Square &oldsq = s.grid(pos_);

  Location newpos = pos_.prev(move_).next(move);
  Square &newsq = s.grid(newpos);
  if(newsq.isWater) return false;

  oldsq.myAnts.erase(std::remove(oldsq.myAnts.begin(), oldsq.myAnts.end(), id_),
                     oldsq.myAnts.end());

  // TODO: resolve battles with enemy ants with this ant removed

  if(oldsq.myAnts.size() == 1) {
    // un-kill the ant on this square
    s.myAnts[oldsq.myAnts[0]].dead_ = false;
  }
  pos_ = newpos;
  move_ = move;
  newsq.myAnts.push_back(id_);
  if(newsq.myAnts.size() != 1) {
    // mark all colliding ants dead
    for(size_t i=0;i<newsq.myAnts.size();i++) {
      s.myAnts[newsq.myAnts[i]].dead_ = false;
    }
  }

  // TODO: resolve battles with enemy ants with this ant added (maybe just by
  // updating the battle strength grid and then doing a final check during eval)

  return true;
}

void Ant::CommitMove(State &s)
{
  if(move_ != -1)
    s.CommitMove(pos_.prev(move_), move_);
}


