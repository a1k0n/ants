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

  Bot();

  double evaluate();

  void playGame();    //plays a single game of Ants

  // returns new score after moving ant
  double iterateAnt(double score, Ant &a);
  void makeMoves();   //makes moves for a single turn
  void endTurn();     //indicates to the engine that it has made its moves
};

#endif //BOT_H_
