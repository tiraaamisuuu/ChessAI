#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include <string>
#include <vector>
#include "Board.hpp"

class AIPlayer {
public:
    AIPlayer(char color);  // 'W' or 'B'

    std::string findBestMove(Board& board);
    double evaluateBoard(const Board& board) const;

private:
    char playerColor;
    std::string lastMoveFrom;

    // Helpers for smarter AI
    std::vector<std::string> generateAllLegalMoves(Board &board, char color);
    double miniMax(Board board, int depth, bool maximizing);
    double pieceValue(char piece) const;

    // Timer tracking
    double totalThinkingTime = 0.0; // cumulative seconds AI has spent thinking
    int movesCount = 0;           // number of moves AI has made
};

#endif
