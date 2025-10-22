#include "AIPlayer.hpp"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <map>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <thread>
#include <chrono>

AIPlayer::AIPlayer(char color) : playerColor(color), lastMoveFrom("") {
    std::srand(std::time(nullptr));
}

std::string AIPlayer::findBestMove(Board& board) {
    std::vector<std::string> legalMoves;

    // Generate all legal moves
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

    if (legalMoves.empty()) return "";

    // Track best move
    double bestScore = -1e9;
    std::string bestMove = "";
    double baseScore = evaluateBoard(board);

    for (const std::string& mv : legalMoves) {
        Board copy = board;
        copy.makeMove(mv);
        double score = evaluateBoard(copy);
        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }

    // Evaluation difference (for commentary)
    double diff = bestScore - baseScore;
    std::string comment;
    if (diff > 1.5) comment = "Excellent!";
    else if (diff > 0.5) comment = "Good move.";
    else if (diff > -0.5) comment = "Okay move.";
    else if (diff > -1.5) comment = "Inaccuracy.";
    else if (diff > -3) comment = "Mistake.";
    else comment = "Blunder!";

    // If no obvious move found, random fallback (avoid repetition)
    if (bestMove.empty()) {
        std::vector<std::string> nonRepeating;
        for (const std::string& mv : legalMoves) {
            std::string from = mv.substr(0, 2);
            if (from != lastMoveFrom)
                nonRepeating.push_back(mv);
        }
        if (nonRepeating.empty()) nonRepeating = legalMoves;
        bestMove = nonRepeating[rand() % nonRepeating.size()];
        comment = "Random fallback.";
    }

    lastMoveFrom = bestMove.substr(0, 2);

    std::cout << "\n========== AI DEBUG INFO ==========\n";
    std::cout << "AI Colour: " << (playerColor == 'W' ? "White" : "Black") << "\n";
    std::cout << "Base Score: " << std::fixed << std::setprecision(2) << baseScore << "\n";
    std::cout << "Chosen Move: " << bestMove << "\n";
    std::cout << "New Score:   " << bestScore << "\n";
    std::cout << "Eval Δ:       " << (bestScore - baseScore) << " → " << comment << "\n";
    std::cout << "Last Move From: " << lastMoveFrom << "\n";
    std::cout << "===================================\n\n";

    return bestMove;
}

double AIPlayer::evaluateBoard(const Board& board) const {
    double score = 0.0;
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

    return (playerColor == 'W') ? score : -score;
}
