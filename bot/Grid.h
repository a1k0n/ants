#ifndef __GRID_H
#define __GRID_H

#include <vector>
#include <iostream>
#include "Location.h"

template<class T>
struct Grid
{
  int rows_, cols_;
  std::vector<std::vector<T> > grid_;

  Grid() {}
  Grid(int rows, int cols):
    rows_(rows), cols_(cols), grid_(rows, std::vector<T>(cols)) {}
  Grid(int rows, int cols, const T& init_value):
    rows_(rows), cols_(cols), grid_(rows, std::vector<T>(cols, init_value)) {}

  void Resize(int rows, int cols) {
    rows_=rows; cols_=cols;
    grid_.resize(rows, std::vector<T>(cols));
  }
  void Init(int rows, int cols, const T& init_value) {
    rows_ = rows; cols_ = cols;
    grid_ = std::vector<std::vector<T> >(rows, std::vector<T>(cols, init_value));
  }

  const T& operator()(int r, int c) const { return grid_[r][c]; }
  const T& operator()(Location l) const { return grid_[l.row][l.col]; }
  T& operator()(int r, int c) { return grid_[r][c]; }
  T& operator()(Location l) { return grid_[l.row][l.col]; }
};

extern std::ostream& operator<<(std::ostream& out, const Grid<int> &grid);

#endif // __GRID_H
