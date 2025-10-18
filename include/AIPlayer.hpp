#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include <string>
#include "Board.hpp"

class AIPlayer {
public:
    AIPlayer(char color);               // 'W' or 'B'
      std::string findBestMove(Board& board);                                      
      double evaluateBoard(const Board& board) const;

private:
    char playerColor;
};

#endif
