// Collapsed Gibbs sampling for tactical solutions

// each ant has a Dirichlet(1) distributin of moves (i suppose we could set the
// Dirichlet parameter to 0 right away if there's water in the way)

// The question is: can we solve for multiple timesteps in advance?
// Can we do it exhaustively without sampling (dynamic programming)?

#include <stdio.h>
#include <stdlib.h>

#include <vector>

using namespace std;

const int kWidth = 5, kHeight = 5;
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
  Ant *pred_; // predecessor ant we are conditionally dependent on
  int team_, x0_, y0_;
  int x_, y_, dir_;
  int dirichlet_[5*5];
  int nEnemies_;
  double damage_;

  Ant(Ant *pred, int team, int x, int y):
    pred_(pred), team_(team), x0_(x), y0_(y) { Init(); }

  void Init() {
    x_ = x0_;
    y_ = y0_;
    for(int j=0;j<5*5;j+=5) {
      if(pred_)
        pred_->Move(j);
      for(int i=0;i<5;i++) {
        // initialize invalid moves to 0 probability
        dirichlet_[j+i] = Move(i) ? kAlpha : 0;
      }
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
    int dir, sum=0;
    bool can_move[5];
    int dir_base = 0;
    if(pred_)
      dir_base = 5*pred_->dir_;
    for(dir = 0;dir<5; dir++) {
      // check whether another ant is in the way
      can_move[dir] = Move(dir);
      if(can_move[dir])
        sum += dirichlet_[dir_base+dir];
    }
    int idx = lrand48() % sum;
    // this is ugly as hell but works
    for(dir = 0;!can_move[dir] || idx >= dirichlet_[dir_base+dir] && dir < 4; dir++) {
      if(can_move[dir])
        idx -= dirichlet_[dir_base+dir];
    }
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
  double score = 0;
  // compute number of enemies for each ant
  for(int i=0;i<ants_.size();i++) {
    ants_[i]->nEnemies_ = 0;
    ants_[i]->damage_ = 0;
  }
  for(int i=0;i<ants_.size() && ants_[i]->team_ == 0;i++) { // assume all player 0 ants are first
    for(int j=4;j<ants_.size();j++) { // FIXME: assume player 1 ants start at 4
      if(distance2(ants_[i], ants_[j]) <= kAttackRadius2) {
        ants_[i]->nEnemies_++;
        ants_[j]->nEnemies_++;
      }
    }
  }

  for(int i=0;i<ants_.size() && ants_[i]->team_ == 0;i++) { // assume all player 0 ants are first
    for(int j=4;j<ants_.size();j++) { // FIXME: assume player 1 ants start at 4
      if(distance2(ants_[i], ants_[j]) <= kAttackRadius2) {
        ants_[i]->damage_ += 1.0/ants_[j]->nEnemies_;
        ants_[j]->damage_ += 1.0/ants_[i]->nEnemies_;
      }
    }
  }

  for(int i=0;i<ants_.size();i++) {
    if(ants_[i]->damage_ >= 1)
      score += ants_[i]->team_ == 0 ? -2 : 1;
  }

  return score;
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
  srand48(time(NULL) + getpid());
  // set up scenario:
  //  01234
  // 0AAAA.    A.AA.   ....A
  // 1.....    A....   AA.a.
  // 2..... -> ..... ->....b
  // 3.....    ..BBB   ..bB.
  // 4..BBB    .....   .....

#if 0
  state.ants_.push_back(new Ant(NULL,           0, 0,0));
  state.ants_.push_back(new Ant(state.ants_[0], 0, 1,0));
  state.ants_.push_back(new Ant(state.ants_[1], 0, 2,0));
  state.ants_.push_back(new Ant(state.ants_[2], 0, 3,0));
  state.ants_.push_back(new Ant(NULL,           1, 2,4));
  state.ants_.push_back(new Ant(state.ants_[4], 1, 3,4));
  state.ants_.push_back(new Ant(state.ants_[5], 1, 4,4));
#else
  state.ants_.push_back(new Ant(NULL,           0, 0,1));
  state.ants_.push_back(new Ant(state.ants_[0], 0, 0,0));
  state.ants_.push_back(new Ant(state.ants_[1], 0, 2,0));
  state.ants_.push_back(new Ant(state.ants_[2], 0, 3,0));
  state.ants_.push_back(new Ant(NULL,           1, 2,3));
  state.ants_.push_back(new Ant(state.ants_[4], 1, 3,3));
  state.ants_.push_back(new Ant(state.ants_[5], 1, 4,3));
#endif

  for(int i=0;i<1000;i++) {
    for(int a=0;a<state.ants_.size();a++) {
      printf("sample %d ant %d ", i, a);
      Ant *ant = state.ants_[a];
      bool minimize = ant->team_ == 0 ? false : true;
      int bestvalue = minimize ? INT_MAX : INT_MIN;
      int bestmoves[5], nbest = 0, ndifferent = 0;
      int lastvalue = bestvalue;
      for(int move=0;move<5;move++) {
        if(ant->Move(move)) {
          int value = state.Value();
          printf("%c=%d ", directions[move][2], value);
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

      int dir_base = 0;
      if(ant->pred_)
        dir_base = 5*ant->pred_->dir_;

      // update dirichlet distribution
      // there should be more than 1 unique score for us to bother updating
      if(ndifferent > 1) {
        for(int d=0;d<nbest;d++)
          ant->dirichlet_[dir_base+bestmoves[d]]++;
      }
      printf("dirichlet(%d)=[", dir_base/5);
      for(int d=0;d<5;d++)
        printf("%c:%d ", directions[d][2], ant->dirichlet_[dir_base+d]);
      printf("\b] ");
      // sample new move for this ant
      int move = ant->SampleMove();
      //if(a == 0) { move = 3; ant->Move(3); }
      int value = state.Value();
      printf("sampled_dir=%c value=%d\n", directions[move][2], value);
    }
  }
  for(int a=0;a<state.ants_.size();a++) {
    Ant *ant = state.ants_[a];
    printf("ant %d (team %d) maximum likelihood: ", a, ant->team_);
    int dir_base = 0;
    int bestvalue = INT_MIN,
        bestmove = 0;
    if(ant->pred_)
      dir_base = 5*ant->pred_->dir_;
    printf("dirichlet(%d)=[", dir_base/5);
    for(int d=0;d<5;d++) {
      int dirparam = ant->dirichlet_[dir_base+d];
      if(dirparam > bestvalue) {
        bestvalue = dirparam;
        bestmove = d;
      }
      printf("%c:%d ", directions[d][2], dirparam);
    }
    printf("\b] ");
    printf("moving %c\n", directions[bestmove][2]);
    ant->Move(bestmove);
  }
  printf("final value = %g (", state.Value());
  for(int a=0;a<state.ants_.size();a++) {
    Ant *ant = state.ants_[a];
    printf("%c", ant->damage_ >= 1.0 ? 'a'+ant->team_ : 'A' + ant->team_);
  }
  printf(")\n");
}
