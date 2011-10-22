#ifndef BOT_H_
#define BOT_H_

#include "State.h"
#include <set>

/*
   This struct represents your bot in the game of Ants
   */
struct Bot
{
  State state;
  std::set<Location> movedAnts;
  std::set<Location> movingAnts;
  std::set<Location> unmovedAnts;

  Bot();

  void playGame();    //plays a single game of Ants

  int moveAnt(Location l);
  void makeMoves();   //makes moves for a single turn
  void endTurn();     //indicates to the engine that it has made its moves
};

#endif //BOT_H_
