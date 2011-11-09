#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

const int kWidth = 4, kHeight = 7;
const int kNAnts1 = 2, kNAnts2 = 1;
const int kNAnts = kNAnts1 + kNAnts2;
const int kNPositions = kWidth*kHeight;
const int kNStates = kNAnts*kNPositions*kNPositions*kNPositions;
const int kAttackRadius2 = 5;

const double kDiscount = 0.7;

double value[kNStates];
bool visiting[kNStates];

int directions[5][3] = {
  {      0, 0, 0}, // don't move
  {-kWidth, 0,-1}, // N
  {      1, 1, 0}, // E
  { kWidth, 0, 1}, // S
  {     -1,-1, 0}  // W
};

// later on, add water squares to the map.  for now we assume the map is
// bounded on all sides; maybe we can make it toroidal later
struct Position
{
  int index_, x_, y_;
  Position(int index): index_(index) { x_ = index_%kWidth; y_ = index_/kWidth; }
  Position(int x, int y): x_(x), y_(y) { index_ = x+y*kWidth; }
  Position() {}

  bool next(int dir) {
    index_ += directions[dir][0];
    x_ += directions[dir][1];
    y_ += directions[dir][2];
    return !(x_ < 0 || x_ >= kWidth || y_ < 0 || y_ >= kHeight);
  }

  bool prev(int dir) {
    index_ -= directions[dir][0];
    x_ -= directions[dir][1];
    y_ -= directions[dir][2];
    return !(x_ < 0 || x_ >= kWidth || y_ < 0 || y_ >= kHeight);
  }

};

Position& operator++(Position &p) {
  p.index_++;
  p.x_++;
  if(p.x_ >= kWidth) {
    p.x_ = 0;
    p.y_ ++;
  }
}

int distance2(const Position &a, const Position &b) {
  int dx = a.x_ - b.x_,
      dy = a.y_ - b.y_;
  return dx*dx + dy*dy;
}

bool operator==(const Position &a, const Position &b) { return a.index_ == b.index_; }

struct State
{
  Position ants_[kNAnts];
  int move_[kNAnts];

  State(Position ants[kNAnts]) { memcpy(ants_, ants, sizeof(ants_)); Init(); }
  State() {
    // not a valid initial state
    memset(ants_, 0, sizeof(ants_));
    Init();
  }

  void Init() { memset(move_, 0, sizeof(move_)); }

  // assuming 2v1 scenario
  bool IsTerminal(double *value) {
    bool a02 = distance2(ants_[0], ants_[2]) <= kAttackRadius2;
    bool a12 = distance2(ants_[1], ants_[2]) <= kAttackRadius2;
    if(a02 && a12) {
      *value = 1;
      return true;
    } else if(a02 || a12) {
      *value = -1;
      return true;
    }
    return false;
  }

  bool ValidState() {
    if(ants_[0] == ants_[1])
      return false;
    if(ants_[0] == ants_[2])
      return false;
    if(ants_[1] == ants_[2])
      return false;
    for(int i=0;i<kNAnts;i++)
      if(ants_[i].x_ < 0 || ants_[i].x_ >= kWidth || ants_[i].y_ < 0 || ants_[i].y_ >= kHeight)
        return false;
    return true;
  }

  bool NextInitialState(int antidx) {
    if(antidx >= kNAnts)
      return false;
    // assume moves are all 0, and just move the ants
    ++ants_[antidx];
    if(ants_[antidx].index_ >= kNPositions) {
      ants_[antidx] = Position(0); // fixme
      if(antidx == 0)
        ants_[0] = Position(ants_[1].index_+1);
      if(!NextInitialState(antidx+1))
        return false;
    }
    return true;
  }

  bool NextInitialState() {
    do {
      if(!NextInitialState(0)) return false;
    } while(!ValidState());
    return true;
  }

  uint32_t index(int ant) {
    return ant + 3*(ants_[0].index_ +
      ants_[1].index_*kNPositions +
      ants_[2].index_*kNPositions*kNPositions);
  }
};

double SolveState(State &s, int antidx, int turn)
{
  double score;
  uint32_t idx = s.index(antidx);
  if(value[idx] != -DBL_MAX)
    return value[idx];

  printf("solving: [%d,%d,%d] a(%d,%d) a(%d,%d) b(%d,%d)\n",
         s.index(antidx), antidx, turn,
         s.ants_[0].x_, s.ants_[0].y_,
         s.ants_[1].x_, s.ants_[1].y_,
         s.ants_[2].x_, s.ants_[2].y_);

  if(antidx == 0) {
    bool terminal = s.IsTerminal(&score);
    if(terminal) {
      printf("         terminal, score=%g\n", score);
      value[idx] = score;
      return score;
    }
    if(visiting[idx])
      return 0; // infinite loop; score tends towards zero
  }

  // avoid loops with "visiting"; in some cases, a stalemate is preferable for
  // one of the players (so score = lim_{n->infinity} kDiscount^n = 0)
  visiting[idx] = true;

  bool minimize = antidx >= kNAnts1;

  // try all moves for ants_[antidx], maximizing value if antidx < kNAnts1,
  // minimizing otherwise
  int bestmove = 0;
  double bestscore = minimize ? DBL_MAX : -DBL_MAX;
  for(int move = 0; move < 5; move++) {
    s.ants_[antidx].next(move);
    int nextant = (antidx+1)%kNAnts;
    if(s.ValidState()) {
      double value;
      if(nextant == 0)
        value = kDiscount * SolveState(s, 0, turn+1);
      else
        value = SolveState(s, nextant, turn);
      if((minimize && value < bestscore) ||
         (!minimize && value > bestscore)) {
        bestscore = value;
        bestmove = move;
      }
    }
    s.ants_[antidx].prev(move);
    assert(s.ValidState());
  }
  visiting[idx] = false;
  value[idx] = bestscore;
  printf("solved: [%d,%d,%d] a(%d,%d) a(%d,%d) b(%d,%d) bestmove=%d score=%g\n",
         s.index(antidx), antidx, turn,
         s.ants_[0].x_, s.ants_[0].y_,
         s.ants_[1].x_, s.ants_[1].y_,
         s.ants_[2].x_, s.ants_[2].y_, bestmove, value[idx]);
  return bestscore;
}

int main()
{
  State s;
  for(int i=0;i<kNStates;i++) {
    value[i] = -DBL_MAX;
    visiting[i] = false;
  }
  while(s.NextInitialState()) {
    bool terminal;
    double score;
    terminal = s.IsTerminal(&score);
    //if(!terminal)
      score = SolveState(s, 0, 0);
    printf("initial: [%d] a(%d,%d) a(%d,%d) b(%d,%d) terminal=%d %g\n",
           s.index(0),
           s.ants_[0].x_, s.ants_[0].y_,
           s.ants_[1].x_, s.ants_[1].y_,
           s.ants_[2].x_, s.ants_[2].y_,
           terminal ? 1:0, score);
  }
}
