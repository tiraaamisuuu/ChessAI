#ifndef AIPLAYER_HPP
#define AIPLAYER_HPP

#include <string>
#include <vector>
#include <unordered_map>

class Board; // forward

class AIPlayer {
public:
    AIPlayer(char color, int maxDepth = 2);  // color = 'W' or 'B'

    // Public API
    std::string findBestMove(Board& board);
    double evaluateBoard(const Board& board) const;

    // Optional: adjust search depth
    void setMaxDepth(int d) { maxDepth = d; }

private:
    char playerColor;
    std::string lastMoveFrom;

    // Search params / stats
    int maxDepth;
    double totalThinkingTime = 0.0;
    int movesCount = 0;

    // Transposition table entry
    struct TTEntry {
        double value;
        int depth; // depth at which value was computed
    };
    std::unordered_map<std::string, TTEntry> tt;

    // Helpers
    double pieceValue(char piece) const;
    std::vector<std::string> generateAllLegalMoves(Board &board, char color) const;
    double alphaBeta(Board &board, int depth, double alpha, double beta, bool maximizing);

    // string key builder for TT
    std::string boardKey(const Board &board) const;
};

#endif
