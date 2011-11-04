#ifndef __FOOD_H
#define __FOOD_H

struct Food
{
  Location pos_;
  bool eaten_;
  int lastUpdate_;
  Food() { eaten_ = false; lastUpdate_ = -1; }
  Food(Location pos):pos_(pos) { eaten_ = false; lastUpdate_ = -1; }
};

#endif
