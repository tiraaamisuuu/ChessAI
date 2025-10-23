#include "AIPlayer.hpp"
#include "Board.hpp"
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <chrono>

AIPlayer::AIPlayer(char color) : playerColor(color), lastMoveFrom(""), totalThinkingTime(0.0), movesCount(0) {
    std::srand(std::time(nullptr));
}

// Piece base values
double AIPlayer::pieceValue(char piece) const {
    switch (std::toupper(piece)) {
        case 'P': return 1.0;
        case 'N': case 'B': return 3.0;
        case 'R': return 5.0;
        case 'Q': return 9.0;
        case 'K': return 1000.0; // king extremely high to prevent losing
    }
    return 0.0;
}

// Evaluate board from AI perspective
double AIPlayer::evaluateBoard(const Board &board) const {
    double score = 0.0;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char p = board.getSquare(x, y);
            if (p == '.') continue;

            double val = pieceValue(p);
            bool isMine = (playerColor == 'W') ? std::isupper(p) : std::islower(p);

            // Base piece value
            score += isMine ? val : -val;

            // Central control bonus
            if ((x == 3 || x == 4) && (y == 3 || y == 4))
                score += isMine ? 0.2 : -0.2;

            // Aggression bonus: check if piece can capture any enemy piece
            for (int ty = 0; ty < 8; ++ty) {
                for (int tx = 0; tx < 8; ++tx) {
                    char target = board.getSquare(tx, ty);
                    if (target == '.') continue;

                    bool isEnemy = (playerColor == 'W') ? std::islower(target) : std::isupper(target);

                    if (isMine && isEnemy) {
                        std::string attempt = std::string() + char('a'+x) + char('8'-y)
                                              + char('a'+tx) + char('8'-ty);
                        if (board.isMoveValid(attempt)) {
                            score += pieceValue(target) * 0.5; // threat bonus
                        }
                    } else if (!isMine && !isEnemy) {
                        std::string attempt = std::string() + char('a'+x) + char('8'-y)
                                              + char('a'+tx) + char('8'-ty);
                        if (board.isMoveValid(attempt)) {
                            score -= pieceValue(target) * 0.5;
                        }
                    }
                }
            }
        }
    }

    return score;
}

// Generate all legal moves for a given color
std::vector<std::string> AIPlayer::generateAllLegalMoves(Board &board, char color) {
    std::vector<std::string> moves;
    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = board.getSquare(fromX, fromY);
            if (piece == '.') continue;
            if ((color == 'W' && !std::isupper(piece)) || (color == 'B' && !std::islower(piece))) continue;

            for (int toY = 0; toY < 8; ++toY) {
                for (int toX = 0; toX < 8; ++toX) {
                    std::string mv = std::string() + char('a'+fromX) + char('8'-fromY)
                                     + char('a'+toX) + char('8'-toY);
                    if (board.isMoveValid(mv)) moves.push_back(mv);
                }
            }
        }
    }
    return moves;
}

// Mini-minimax 2-ply
double AIPlayer::miniMax(Board board, int depth, bool maximizing) {
    if (depth == 0) return evaluateBoard(board);

    char color = maximizing ? playerColor : (playerColor=='W' ? 'B' : 'W');
    std::vector<std::string> moves = generateAllLegalMoves(board, color);
    if (moves.empty()) return evaluateBoard(board);

    double best = maximizing ? -1e9 : 1e9;
    for (auto &mv : moves) {
        Board copy = board;
        copy.makeMove(mv);

        double score = miniMax(copy, depth-1, !maximizing);

        // Aggression bonus for captures
        char captured = board.getSquare(mv[2]-'a', '8'-mv[3]);
        if (captured != '.') score += (maximizing ? 1 : -1) * pieceValue(captured) * 0.8;

        if (maximizing) best = std::max(best, score);
        else best = std::min(best, score);
    }

    return best;
}

// Main AI function
std::string AIPlayer::findBestMove(Board &board) {
    auto start = std::chrono::high_resolution_clock::now(); // Start timer

    std::vector<std::string> legalMoves = generateAllLegalMoves(board, playerColor);
    if (legalMoves.empty()) return "";

    double bestScore = -1e9;
    std::string bestMove = "";
    double baseScore = evaluateBoard(board);

    for (auto &mv : legalMoves) {
        Board copy = board;
        copy.makeMove(mv);

        double score = miniMax(copy, 3, false); // 3-ply lookahead

        // Extra aggression bonus
        char toPiece = board.getSquare(mv[2]-'a', '8'-mv[3]);
        if (toPiece != '.') score += pieceValue(toPiece) * 0.8;

        // Small random bias for variety
        char movingPiece = board.getSquare(mv[0]-'a', '8'-mv[1]);
        double bias = 0.0;
        switch (std::toupper(movingPiece)) {
            case 'P': bias = ((rand()%100)<20)?0.2:0.0; break;
            case 'N': bias = ((rand()%100)<15)?0.3:0.0; break;
            case 'B': bias = ((rand()%100)<10)?0.3:0.0; break;
            case 'R': bias = ((rand()%100)<5)?0.4:0.0; break;
            case 'Q': bias = ((rand()%100)<5)?0.5:0.0; break;
            case 'K': bias = -1.0; break;
        }
        score += bias;

        if (score > bestScore) {
            bestScore = score;
            bestMove = mv;
        }
    }

    // Commentary
    double diff = bestScore - baseScore;
    std::string comment;
    if (diff > 1.5) comment = "Excellent!";
    else if (diff > 0.5) comment = "Good move.";
    else if (diff > -0.5) comment = "Okay move.";
    else if (diff > -1.5) comment = "Inaccuracy.";
    else if (diff > -3) comment = "Mistake.";
    else comment = "Blunder!";

    if (bestMove.empty()) {
        std::vector<std::string> nonRepeating;
        for (auto &mv : legalMoves)
            if (mv.substr(0,2) != lastMoveFrom) nonRepeating.push_back(mv);
        if (nonRepeating.empty()) nonRepeating = legalMoves;
        bestMove = nonRepeating[rand()%nonRepeating.size()];
        comment = "Random fallback.";
    }

    lastMoveFrom = bestMove.substr(0,2);

    auto end = std::chrono::high_resolution_clock::now(); // End timer
    std::chrono::duration<double> elapsed = end - start;

    totalThinkingTime += elapsed.count();
    movesCount++;

    std::cout << "\n========== AI DEBUG INFO ==========\n";
    std::cout << "AI Colour: " << (playerColor=='W'?"White":"Black") << "\n";
    std::cout << "Chosen Move: " << bestMove << "\n";
    std::cout << "Move Eval Δ: " << diff << " → " << comment << "\n";
    std::cout << "Time for this move: " << elapsed.count()*1000 << " ms\n";
    if (movesCount > 0)
        std::cout << "Average Thinking Time: " << (totalThinkingTime/movesCount*1000) << " ms\n";
    std::cout << "===================================\n\n" << std::flush;

    return bestMove;
}
