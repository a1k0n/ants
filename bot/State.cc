#include "Score.h"
#include "State.h"

#include <limits.h>
#include <queue>

using namespace std;

int Location::rows = 0;
int Location::cols = 0;

//constructor
State::State()
{
  gameover = 0;
  turn = 0;
}

//deconstructor
State::~State()
{
}

//sets the state up
void State::setup()
{
  grid.Resize(rows, cols);
}

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  for(int row=0; row<rows; row++)
    for(int col=0; col<cols; col++)
      if(!grid(row,col).isWater)
        grid(row,col).reset();
}

//outputs move information to the engine
void State::CommitMove(const Location &loc, int direction)
{
  cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;
}

//returns the euclidean distance between two locations with the edges wrapped
double State::distance2(const Location &loc1, const Location &loc2)
{
  int d1 = abs(loc1.row-loc2.row),
      d2 = abs(loc1.col-loc2.col),
      dr = min(d1, rows-d1),
      dc = min(d2, cols-d2);
  return dr*dr + dc*dc;
}

// Generate a vector of relative locations where visibility increases and
// decreases
void State::computeCircleDelta(const Location &delta,
                               vector<pair<Location, int> > *adjust)
{
  for(int y = -viewBoxSize-1; y <= viewBoxSize+1; y++) {
    for(int x = -viewBoxSize-1; x <= viewBoxSize+1; x++) {
      int dx = x-delta.col,
          dy = y-delta.row;
      int delta = (((dx*dx + dy*dy < viewradius2) ? 1 : 0) -
                   ((x*x + y*y < viewradius2) ? 1 : 0));
      if(delta != 0)
        adjust->push_back(make_pair(Location(y,x), delta));
    }
  }
}

void State::updateAntVisibility(Ant &a)
{
  const Location &l = a.pos_;
  int x0 = wrapCol(l.col - viewBoxSize),
      y0 = wrapRow(l.row - viewBoxSize);
  int r, c, dx, dy;
  for(c = x0, dx = -viewBoxSize; dx <= viewBoxSize; dx++, c = c == cols-1 ? 0:c+1) {
    for(r = y0, dy = -viewBoxSize; dy <= viewBoxSize; dy++, r = r == rows-1 ? 0:r+1) {
      if(dx*dx + dy*dy <= viewradius2) {
        grid(r,c).visibility ++;
        grid(r,c).isExplored = true;
        grid(r,c).lastSeen = turn;
      }
    }
  }
}

void State::updateVisionInformation()
{
  evalScore = 0;
  for(int a=0; a<(int) myAnts.size(); a++) {
    updateAntVisibility(myAnts[a]);
  }
}

// requires updated visibility information
void State::updateStateEstimate()
{
  // delete any visible enemy hills which are no longer present
  set<Location>::iterator i = enemyHills.begin();
  while(i != enemyHills.end()) {
    set<Location>::iterator cur_it = i++;
    const Square &s = grid(*cur_it);
    if(s.visibility && !s.isHill)
      enemyHills.erase(cur_it);
  }

  // delete any visible food which has disappeared
  map<Location, Food>::iterator j = food.begin();
  while(j != food.end()) {
    map<Location, Food>::iterator cur_it = j++;
    const Square &s = grid(cur_it->first);
    if(s.visibility && !s.isFood)
      food.erase(cur_it);
  }
}

void State::updateDistanceInformation()
{
  // compute pathfinding info for food, hills, etc
  vector<Location> seed;
  for(int i = 0; i < myAnts.size(); ++i)
    seed.push_back(myAnts[i].pos_);
  bfs(seed, Square::DIST_MY_ANTS);

  seed.clear();
  for(set<Location>::iterator i = enemyHills.begin();
      i != enemyHills.end(); ++i)
    seed.push_back(*i);
  bfs(seed, Square::DIST_ENEMY_HILLS);
}

// do full BFS path from all (FIXME) ants (or whatever else)
// and update exploration score as necessary
void State::bfs(vector<Location> seed, int type)
{
  // first, clear the grid of our given type
  for(int r=0;r<rows;r++) {
    for(int c=0;c<cols;c++) {
      grid(r,c).distance[type] = INT_MAX;
    }
  }
  int dist = 0;
  vector<Location> queue;
  while(!seed.empty()) {
    for(size_t i=0;i<seed.size();i++) {
      const Location &l = seed[i];
      Square &sq = grid(l);
      if(sq.distance[type] != INT_MAX) continue;
      sq.distance[type] = dist;
      evalScore += SquareScore(*this, sq);
      for(int d=0;d<4;d++) {
        Location next = l.next(d);
        if(grid(next).isWater) continue;
        if(grid(next).distance[type] != INT_MAX) continue;
        queue.push_back(next);
      }
    }
    seed = queue;
    queue.clear();
    dist++;
  }
}

void State::dumpDistances(int type)
{
  for(int r=0;r<rows;r++) {
    for(int c=0;c<cols;c++) {
      int dist = grid(r,c).distance[type];
      if(dist == INT_MAX)
        cerr << " ";
      else
        cerr << min(9, grid(r,c).distance[type]);
    }
    cerr << endl;
  }
}

struct DijkstraCompare
{
  bool operator()(const pair<int, Location> &a,
                  const pair<int, Location> &b) {
    return a.first == b.first ? a.second < b.second :
      b.first < a.first;
  }
};

void State::updateAntPos(const Location &oldpos, const Location &newpos)
{
  if(oldpos == newpos)
    return;
  // so, first we have to go through and eradicate everything adjacent to the
  // old position such that the distance is strictly increasing, and at the
  // boundary, add squares to the priority queue
  int dist = 0; // we'll use the same conventions as above here
  priority_queue<pair<int, Location>, vector<pair<int, Location> >,
    DijkstraCompare> dqueue;
  vector<Location> seed, queue;
  seed.push_back(oldpos);
  while(!seed.empty()) {
    for(size_t i=0;i<seed.size();i++) {
      const Location &l = seed[i];
      Square &sq = grid(l);
      if(sq.distance[Square::DIST_MY_ANTS] == INT_MAX) continue;
      if(sq.distance[Square::DIST_MY_ANTS] == dist) {
        for(int d=0;d<4;d++) {
          Location next = l.next(d);
          if(grid(next).isWater) continue;
          if(grid(next).distance[Square::DIST_MY_ANTS] == INT_MAX) continue;
          queue.push_back(next);
        }
      } else {
        dqueue.push(make_pair(grid(l).distance[Square::DIST_MY_ANTS], l));
      }
      evalScore -= SquareScore(*this, sq);
      sq.distance[Square::DIST_MY_ANTS] = INT_MAX;
    }
    seed = queue;
    queue.clear();
    dist++;
  }

  // add in our new ant's position
  dqueue.push(make_pair(0, newpos));
  // okay, now we have a priority queue full of our boundary and our new ant's
  // position, so we just run dijkstra's
  while(!dqueue.empty()) {
    int dist = dqueue.top().first;
    Location l = dqueue.top().second;
    dqueue.pop();
    Square &sq = grid(l);
    if(sq.distance[Square::DIST_MY_ANTS] != INT_MAX)
      continue;
    sq.distance[Square::DIST_MY_ANTS] = dist;
    evalScore += SquareScore(*this, sq);
    for(int d=0;d<4;d++) {
      Location next = l.next(d);
      if(grid(next).isWater) continue;
      if(grid(next).distance[Square::DIST_MY_ANTS] != INT_MAX) continue;
      dqueue.push(make_pair(dist+1, next));
    }
  }
  fprintf(stderr, "updated ant pos %d,%d -> %d,%d (score=%g)\n",
          oldpos.col, oldpos.row,
          newpos.col, newpos.row,
          evalScore);
}

/*
   This is the output function for a state. It will add a char map
   representation of the state to the output stream passed to it.
 */
ostream& operator<<(ostream &os, const State &state)
{
  for(int row=0; row<state.rows; row++)
  {
    for(int col=0; col<state.cols; col++)
    {
      const Square &s = state.grid(row,col);
      if(s.isWater)
        os << '%';
      else if(s.isFood)
        os << '*';
      else if(s.isHill)
        os << (char)('A' + s.hillPlayer);
      else if(s.ant >= 0)
        os << (char)('a' + s.ant);
      else if(s.visibility)
        os << '.';
      else if(s.isExplored)
        os << '/';
      else
        os << '?';
    }
    os << endl;
  }

#if 0
  for(int row=0; row<state.rows; row++)
  {
    for(int col=0; col<state.cols; col++)
    {
      const Square &s = state.grid(row,col);
      char poop[4096];
      int d = s.distance[Square::DIST_FRONTIER];
      sprintf(poop, "%2d", d == INT_MAX ? -1 : d);
      os << poop;
    }
    os << endl;
  }
#endif

  return os;
}

void State::setViewRadius(int radius2)
{
  viewradius2 = radius2;
  viewBoxSize = (int) sqrt(radius2);
  for(int i=0;i<TDIRECTIONS;i++) {
    visibilityAdjust[i].clear();
    computeCircleDelta(Location(DIRECTIONS[i][0], DIRECTIONS[i][1]),
                       &visibilityAdjust[i]);
    fprintf(stderr, "direction %c: adjust ", CDIRECTIONS[i]);
    for(int j=0;j<visibilityAdjust[i].size();j++) {
      const pair<Location, int> &adj = visibilityAdjust[i][j];
      fprintf(stderr, "(%d,%d,%+d) ", adj.first.col, adj.first.row, adj.second);
    }
    fprintf(stderr, "\n");
  }
}


//input function
istream& operator>>(istream &is, State &state)
{
  int row, col, player;
  string inputType, junk;

  //finds out which turn it is
  while(is >> inputType)
  {
    if(inputType == "end")
    {
      state.gameover = 1;
      break;
    }
    else if(inputType == "turn")
    {
      is >> state.turn;
      break;
    }
    else //unknown line
      getline(is, junk);
  }

  if(state.turn == 0)
  {
    //reads game parameters
    while(is >> inputType)
    {
      if(inputType == "loadtime") {
        is >> state.loadtime;
      } else if(inputType == "turntime") {
        is >> state.turntime;
      } else if(inputType == "rows") {
        is >> state.rows;
        Location::rows = state.rows;
      } else if(inputType == "cols") {
        is >> state.cols;
        Location::cols = state.cols;
      } else if(inputType == "turns") {
        is >> state.turns;
      } else if(inputType == "viewradius2")
      {
        int radius2;
        is >> radius2;
        if(radius2 != state.viewradius2)
          state.setViewRadius(radius2);
      }
      else if(inputType == "attackradius2")
      {
        is >> state.attackradius2;
      }
      else if(inputType == "spawnradius2")
      {
        is >> state.spawnradius2;
      }
      else if(inputType == "ready") //end of parameter input
      {
        state.timer.start();
        break;
      }
      else    //unknown line
        getline(is, junk);
    }
  }
  else
  {
    //reads information about the current turn
    while(is >> inputType)
    {
      if(inputType == "w") //water square
      {
        is >> row >> col;
        state.grid(row,col).isWater = 1;
      }
      else if(inputType == "f") //food square
      {
        is >> row >> col;
        Location l(row,col);
        state.grid(l).isFood = 1;
        state.food.insert(make_pair(l, Food(l)));
      }
      else if(inputType == "a") //live ant square
      {
        is >> row >> col >> player;
        Location l(row,col);
        Square &sq = state.grid(l);
        sq.ant = player;
        if(player == 0) {
          int id = state.myAnts.size();
          sq.myAnts.push_back(id);
          state.myAnts.push_back(Ant(id, l));
        } else {
          int id = state.enemyAnts.size();
          sq.enemyAnts.push_back(id);
          state.enemyAnts.push_back(Ant(id, l));
        }
      }
      else if(inputType == "d") //dead ant square
      {
        is >> row >> col >> player;
        //state.grid(row,col).deadAnts.push_back(player);
      }
      else if(inputType == "h")
      {
        is >> row >> col >> player;
        state.grid(row,col).isHill = 1;
        state.grid(row,col).hillPlayer = player;
        if(player == 0)
          state.myHills.insert(Location(row, col));
        else
          state.enemyHills.insert(Location(row, col));

      }
      else if(inputType == "players") //player information
        is >> state.noPlayers;
      else if(inputType == "scores") //score information
      {
        state.scores = vector<double>(state.noPlayers, 0.0);
        for(int p=0; p<state.noPlayers; p++)
          is >> state.scores[p];
      }
      else if(inputType == "go") //end of turn input
      {
        if(state.gameover)
          is.setstate(std::ios::failbit);
        else
          state.timer.start();
        break;
      }
      else //unknown line
        getline(is, junk);
    }
  }

  return is;
}
