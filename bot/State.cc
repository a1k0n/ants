#include "State.h"
#include <limits.h>

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
  grid_ = vector<vector<Square> >(rows, vector<Square>(cols, Square()));
}

//resets all non-water squares to land and clears the bots ant vector
void State::reset()
{
  myAnts.clear();
  enemyAnts.clear();
  myHills.clear();
  enemyHills.clear();
  food.clear();
  for(int row=0; row<rows; row++)
    for(int col=0; col<cols; col++)
      if(!grid(row,col).isWater)
        grid(row,col).reset();
}

//outputs move information to the engine
void State::makeMove(const Location &loc, int direction)
{
  cout << "o " << loc.row << " " << loc.col << " " << CDIRECTIONS[direction] << endl;

  // NOTE: the myAnts array needs to be synchronized separately if we do this
  Location nLoc = loc.next(direction);
  grid(nLoc).ant = grid(loc).ant;
  grid(loc).ant = -1;
}

//returns the euclidean distance between two locations with the edges wrapped
double State::distance(const Location &loc1, const Location &loc2)
{
  int d1 = abs(loc1.row-loc2.row),
      d2 = abs(loc1.col-loc2.col),
      dr = min(d1, rows-d1),
      dc = min(d2, cols-d2);
  return sqrt(dr*dr + dc*dc);
}

/*
   This function will update update the lastSeen value for any squares currently
   visible by one of your live ants.

   BE VERY CAREFUL IF YOU ARE GOING TO TRY AND MAKE THIS FUNCTION MORE EFFICIENT,
   THE OBVIOUS WAY OF TRYING TO IMPROVE IT BREAKS USING THE EUCLIDEAN METRIC, FOR
   A CORRECT MORE EFFICIENT IMPLEMENTATION, TAKE A LOOK AT THE GET_VISION FUNCTION
   IN ANTS.PY ON THE CONTESTS GITHUB PAGE.
   */
void State::updateVisionInformation()
{
  std::queue<Location> locQueue;
  Location sLoc, cLoc, nLoc;

  for(int a=0; a<(int) myAnts.size(); a++)
  {
    sLoc = myAnts[a];
    locQueue.push(sLoc);

    std::vector<std::vector<bool> > visited(rows, std::vector<bool>(cols, 0));
    grid(sLoc).isVisible = 1;
    grid(sLoc).isExplored = 1;
    visited[sLoc.row][sLoc.col] = 1;

    while(!locQueue.empty())
    {
      cLoc = locQueue.front();
      locQueue.pop();

      for(int d=0; d<TDIRECTIONS; d++)
      {
        nLoc = cLoc.next(d);

        if(!visited[nLoc.row][nLoc.col] && distance(sLoc, nLoc) <= viewradius)
        {
          grid(nLoc).isVisible = 1;
          grid(nLoc).isExplored = 1;
          locQueue.push(nLoc);
        }
        visited[nLoc.row][nLoc.col] = 1;
      }
    }
  }
}

void State::updateDistanceInformation()
{
  // compute pathfinding info for food, hills, etc
  bfs(food, Square::DIST_FOOD);
  bfs(enemyAnts, Square::DIST_ENEMY_ANTS);
  bfs(myHills, Square::DIST_OUR_HILL);
  bfs(enemyHills, Square::DIST_ENEMY_HILL);

  // compute pathfinding info for frontier by starting with all invisible squares
  std::vector<Location> frontier;
  for(int r=0;r<rows;r++) {
    for(int c=0;c<cols;c++) {
      if(!grid(r,c).isExplored && !grid(r,c).isWater) {
        frontier.push_back(Location(r,c));
      }
    }
  }
  bfs(frontier, Square::DIST_FRONTIER);
}

void State::bfs(std::vector<Location> seed, int type)
{
  // first, clear the grid of our given type
  for(int r=0;r<rows;r++) {
    for(int c=0;c<cols;c++) {
      grid(r,c).distance[type] = INT_MAX;
    }
  }
  int dist = 0;
  std::vector<Location> queue;
  while(!seed.empty()) {
    for(size_t i=0;i<seed.size();i++) {
      Location l = seed[i];
      if(grid(l).distance[type] != INT_MAX) continue;
      grid(l).distance[type] = dist;
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

/*
   This is the output function for a state. It will add a char map
   representation of the state to the output stream passed to it.

   For example, you might call "cout << state << endl;"
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
      else if(s.isVisible)
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
        is >> state.viewradius;
        state.viewradius = sqrt(state.viewradius);
      }
      else if(inputType == "attackradius2")
      {
        is >> state.attackradius;
        state.attackradius = sqrt(state.attackradius);
      }
      else if(inputType == "spawnradius2")
      {
        is >> state.spawnradius;
        state.spawnradius = sqrt(state.spawnradius);
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
        state.grid(row,col).isFood = 1;
        state.food.push_back(Location(row, col));
      }
      else if(inputType == "a") //live ant square
      {
        is >> row >> col >> player;
        state.grid(row,col).ant = player;
        if(player == 0)
          state.myAnts.push_back(Location(row, col));
        else
          state.enemyAnts.push_back(Location(row, col));
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
          state.myHills.push_back(Location(row, col));
        else
          state.enemyHills.push_back(Location(row, col));

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
