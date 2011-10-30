#ifndef LOCATION_H_
#define LOCATION_H_

// constants
const int TDIRECTIONS = 4;
const char CDIRECTIONS[4] = {'N', 'E', 'S', 'W'};
const int DIRECTIONS[4][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1} };      //{N, E, S, W}

// struct for representing locations in the grid.
struct Location
{
  int row, col;
  static int rows, cols;

  Location() {
    row = col = 0;
  }

  Location(int r, int c) {
    row = r;
    col = c;
  }

  Location next(int direction) const {
    if(direction == -1) return *this;
    return Location((row + DIRECTIONS[direction][0] + rows) % rows,
                    (col + DIRECTIONS[direction][1] + cols) % cols );
  }
  Location prev(int direction) const {
    if(direction == -1) return *this;
    return Location((row - DIRECTIONS[direction][0] + rows) % rows,
                    (col - DIRECTIONS[direction][1] + cols) % cols );
  }
};

static bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

static bool operator<(const Location &a, const Location &b) {
  return a.row == b.row ? a.col < b.col : a.row < b.row;
}

static Location operator+(const Location &a, const Location &b) {
  return Location(a.row+b.row, a.col+b.col);
}

#endif //LOCATION_H_
