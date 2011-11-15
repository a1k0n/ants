#include "Grid.h"

#include <iostream>
#include <iomanip>
#include <limits.h>

using namespace std;

ostream& operator<<(ostream& out, const Grid<int> &grid)
{
  out << grid.cols_ << "x" << grid.rows_ << endl;
  for(int j=0;j<grid.rows_;j++) {
    for(int i=0;i<grid.cols_;i++) {
      int dist = grid(j,i);
      if(dist == INT_MAX)
        out << " ";
      else if(dist > 9)
        out << "9";
      else
        out << grid(j,i);
    }
    out << endl;
  }
  return out;
}
