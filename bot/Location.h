#ifndef LOCATION_H_
#define LOCATION_H_

// constants
const int TDIRECTIONS = 5;
const char CDIRECTIONS[5] = {
  '-',   'N',     'E',    'S',    'W'};
const int DIRECTIONS[5][2] = {
  {0,0}, {-1, 0}, {0, 1}, {1, 0}, {0, -1} };

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
    return Location((row + DIRECTIONS[direction][0] + rows) % rows,
                    (col + DIRECTIONS[direction][1] + cols) % cols );
  }
  Location prev(int direction) const {
    return Location((row - DIRECTIONS[direction][0] + rows) % rows,
                    (col - DIRECTIONS[direction][1] + cols) % cols );
  }
};

static inline bool operator==(const Location &a, const Location &b) {
  return a.row == b.row && a.col == b.col;
}

static inline bool operator<(const Location &a, const Location &b) {
  return a.row == b.row ? a.col < b.col : a.row < b.row;
}

static inline Location operator+(const Location &a, const Location &b) {
  return Location((a.row+b.row+Location::rows)%Location::rows,
                  (a.col+b.col+Location::cols)%Location::cols);
}

#endif //LOCATION_H_
