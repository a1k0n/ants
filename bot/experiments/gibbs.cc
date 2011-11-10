// Collapsed Gibbs sampling for tactical solutions

// each ant has a Dirichlet(1) distributin of moves (i suppose we could set the
// Dirichlet parameter to 0 right away if there's water in the way)

// The question is: can we solve for multiple timesteps in advance?
// Can we do it exhaustively without sampling (dynamic programming)?

#include <stdio.h>
#include <stdlib.h>

#include <vector>

using namespace std;

const int kWidth = 3, kHeight = 4;
const int kNAnts1 = 2, kNAnts2 = 1;
const int kNAnts = kNAnts1 + kNAnts2;
const int kNPositions = kWidth*kHeight;
const int kAttackRadius2 = 5;

const int kAlpha = 1; // alpha parameter of dirichlet distribution

int directions[5][3] = {
  { 0, 0, '-'},
  { 0,-1, 'N'},
  { 1, 0, 'E'},
  { 0, 1, 'S'},
  {-1, 0, 'W'}
};

struct Ant;
struct State
{
  vector<Ant*> ants_;

  bool IsColliding(Ant *a);
  double Value();
} state;

struct Ant
{
  int team_, x0_, y0_;
  int x_, y_, dir_;
  int dirichlet_[5];

  Ant(int team, int x, int y): team_(team), x0_(x), y0_(y) { Init(); }
  void Init() {
    x_ = x0_;
    y_ = y0_;
    for(int i=0;i<5;i++) {
      // initialize invalid moves to 0 probability
      dirichlet_[i] = Move(i) ? kAlpha : 0;
    }
  }

  bool Move(int dir) {
    x_ = x0_ + directions[dir][0];
    y_ = y0_ + directions[dir][1];
    dir_ = dir;
    return !(x_ < 0 || x_ >= kWidth ||
             y_ < 0 || y_ >= kHeight ||
             state.IsColliding(this));
  }

  int SampleMove(void) {
    int dir, sum;
    for(dir = 0;dir<5; dir++) {
      // check whether another ant is in the way
      if(Move(dir))
        sum += dirichlet_[dir];
    }
    int idx = lrand48() % sum;
    for(dir = 0;idx >= dirichlet_[dir] && dir < 4; dir++)
      idx -= dirichlet_[dir];
    Move(dir);
    return dir;
  }
};

int distance2(const Ant *a, const Ant *b) {
  int dx = a->x_ - b->x_,
      dy = a->y_ - b->y_;
  return dx*dx + dy*dy;
}

// returns 0 if unknown, nonzero if favorable one way or another
double State::Value() {
  // here we assume for simplicity a 3-ant scenario
  bool a02 = distance2(ants_[0], ants_[2]) <= kAttackRadius2;
  bool a12 = distance2(ants_[1], ants_[2]) <= kAttackRadius2;
  if(a02 && a12) {
    return 1;
  } else if(a02 || a12) {
    return -1;
  }
  return 0;
}

bool State::IsColliding(Ant *a)
{
  for(size_t i=0;i<ants_.size();i++) {
    if(ants_[i] == a) continue;
    if(ants_[i]->x_ == a->x_ &&
       ants_[i]->y_ == a->y_ &&
       ants_[i]->team_ == a->team_)
      return true;
  }
  return false;
}

int main()
{
  // set up scenario:
  // .A.       ...
  // A..    -> .A.
  // ...       A..
  // ..B       .b.

  state.ants_.push_back(new Ant(0, 1,0));
  state.ants_.push_back(new Ant(0, 0,1));
  state.ants_.push_back(new Ant(1, 2,3));

  for(int i=0;i<1000;i++) {
    for(int a=0;a<state.ants_.size();a++) {
      printf("sample %d ant %d ", i, a);
      Ant *ant = state.ants_[a];
      bool minimize = ant->team_ == 0 ? false : true;
      int bestvalue = minimize ? INT_MAX : INT_MIN;
      int bestmoves[5], nbest = 0;
      for(int move=0;move<5;move++) {
        if(ant->Move(move)) {
          int value = state.Value();
          printf("%c=%d ", directions[move][2], value);
          if((minimize && value < bestvalue && value < 0) ||
             (!minimize && value > bestvalue && value > 0)) {
            nbest = 0;
            bestvalue = value;
            bestmoves[nbest++] = move;
          } else if(value == bestvalue) {
            bestmoves[nbest++] = move;
          }
        }
      }
      // update dirichlet distribution
      printf("dirichlet=[");
      for(int d=0;d<5;d++)
        printf("%c:%d ", directions[d][2], ant->dirichlet_[d]);
      printf("\b] ");
      for(int d=0;d<nbest;d++)
        ant->dirichlet_[bestmoves[d]]++;
      // sample new move for this ant
      int move = ant->SampleMove();
      int value = state.Value();
      printf("sampled_dir=%c value=%d\n", directions[move][2], value);
    }
  }
}
