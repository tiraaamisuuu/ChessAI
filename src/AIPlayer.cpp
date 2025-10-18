#include "AIPlayer.hpp"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>
#include <cctype> // for std::isupper / std::islower

AIPlayer::AIPlayer(char color) : playerColor(color) {
    std::srand(std::time(nullptr));
}

// Step 1: Random-move bot (still same behaviour)
std::string AIPlayer::findBestMove(Board& board) {
    std::vector<std::string> legalMoves;

    // 1.Generate all legal moves
    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = board.getSquare(fromX, fromY);
            if (piece == '.') continue;
            if ((playerColor == 'W' && !std::isupper(piece)) ||
                (playerColor == 'B' && !std::islower(piece))) continue;

            for (int toY = 0; toY < 8; ++toY) {
                for (int toX = 0; toX < 8; ++toX) {
                    std::string mv = std::string() + char('a' + fromX) + char('8' - fromY)
                                     + char('a' + toX) + char('8' - toY);
                    if (board.isMoveValid(mv))
                        legalMoves.push_back(mv);
                }
            }
        }
    }

    if (legalMoves.empty()) return ""; // No legal move

    // 2.Evaluate each move and pick the best
    double bestScore = -1e9;
    std::string bestMove = legalMoves[0];

    for (const std::string& mv : legalMoves) {
        Board copy = board;       // Make a copy
        copy.makeMove(mv);        // Simulate the move
        double score = evaluateBoard(copy);  // Evaluate resulting board
        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }

    return bestMove;
}


// Step 2: Simple board evaluation (material-based)
double AIPlayer::evaluateBoard(const Board& board) const {
    double score = 0.0;

    // Piece values (positive for White, negative for Black)
    std::map<char, double> pieceValues = {
        {'P', 1.0}, {'N', 3.0}, {'B', 3.0}, {'R', 5.0}, {'Q', 9.0}, {'K', 200.0},
        {'p', -1.0}, {'n', -3.0}, {'b', -3.0}, {'r', -5.0}, {'q', -9.0}, {'k', -200.0}
    };

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char piece = board.getSquare(x, y);
            if (pieceValues.count(piece))
                score += pieceValues[piece];
        }
    }

    // Return from AI’s perspective (positive = AI is winning)
    return (playerColor == 'W') ? score : -score;
}
