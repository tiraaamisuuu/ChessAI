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
#include <limits>

AIPlayer::AIPlayer(char color, int maxDepth_)
    : playerColor(color), lastMoveFrom(""), maxDepth(maxDepth_) {
    std::srand(std::time(nullptr));
}

// small piece values
double AIPlayer::pieceValue(char piece) const {
    switch (std::toupper(static_cast<unsigned char>(piece))) {
        case 'P': return 1.0;
        case 'N': return 3.0;
        case 'B': return 3.0;
        case 'R': return 5.0;
        case 'Q': return 9.0;
        case 'K': return 1000.0;
    }
    return 0.0;
}

// Keep your evaluation (called by alphaBeta leafs)
double AIPlayer::evaluateBoard(const Board &board) const {
    // You already had an evaluateBoard function in your project.
    // Keep this consistent with your previous implementation or extend it here.
    // For now use a simple material + small center bonus + threat bonus as before.

    double score = 0.0;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            char p = board.getSquare(x, y);
            if (p == '.') continue;

            double val = pieceValue(p);
            bool isMine = (playerColor == 'W') ? std::isupper(static_cast<unsigned char>(p))
                                               : std::islower(static_cast<unsigned char>(p));

            // material
            score += isMine ? val : -val;

            // central control tiny bonus
            if ((x==3||x==4) && (y==3||y==4)) score += isMine ? 0.15 : -0.15;

            // threat bonus: if this piece has a legal capture on opponent piece, reward
            for (int ty = 0; ty < 8; ++ty) {
                for (int tx = 0; tx < 8; ++tx) {
                    char target = board.getSquare(tx, ty);
                    if (target == '.') continue;
                    bool targetIsEnemy = (playerColor == 'W') ? std::islower(static_cast<unsigned char>(target))
                                                              : std::isupper(static_cast<unsigned char>(target));
                    if (isMine && targetIsEnemy) {
                        std::string attempt = std::string() + char('a'+x) + char('8'-y)
                                              + char('a'+tx) + char('8'-ty);
                        if (board.isMoveValid(attempt)) {
                            score += pieceValue(target) * 0.35; // threat bonus
                        }
                    } else if (!isMine && !targetIsEnemy) {
                        std::string attempt = std::string() + char('a'+x) + char('8'-y)
                                              + char('a'+tx) + char('8'-ty);
                        if (board.isMoveValid(attempt)) {
                            score -= pieceValue(target) * 0.35;
                        }
                    }
                }
            }
        }
    }

    // return from AI's perspective (positive => good for AI)
    return (playerColor == 'W') ? score : -score;
}

// Build a simple ASCII key for the board + side to move; OK for a TT prototype
std::string AIPlayer::boardKey(const Board &board) const {
    std::string k;
    k.reserve(65 + 1);
    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            k.push_back(board.getSquare(x, y));
    k.push_back(board.getCurrentPlayer()); // side-to-move matters
    return k;
}

// Generate all legal moves for given colour
std::vector<std::string> AIPlayer::generateAllLegalMoves(Board &board, char color) const {
    std::vector<std::string> moves;
    for (int fromY=0; fromY<8; ++fromY) {
        for (int fromX=0; fromX<8; ++fromX) {
            char piece = board.getSquare(fromX, fromY);
            if (piece == '.') continue;
            if ((color=='W' && !std::isupper(static_cast<unsigned char>(piece))) ||
                (color=='B' && !std::islower(static_cast<unsigned char>(piece)))) continue;

            for (int toY=0; toY<8; ++toY) {
                for (int toX=0; toX<8; ++toX) {
                    std::string mv = std::string() + char('a'+fromX) + char('8'-fromY)
                                     + char('a'+toX) + char('8'-toY);
                    if (board.isMoveValid(mv)) moves.push_back(mv);
                }
            }
        }
    }
    return moves;
}

// Alpha-beta with TT and move ordering (captures first)
double AIPlayer::alphaBeta(Board &board, int depth, double alpha, double beta, bool maximizing) {
    // terminal or depth 0 => eval
    if (depth == 0) return evaluateBoard(board);

    // TT lookup
    std::string key = boardKey(board);
    auto it = tt.find(key);
    if (it != tt.end() && it->second.depth >= depth) {
        // cached value at same-or-deeper depth — use it
        return it->second.value;
    }

    char color = maximizing ? playerColor : (playerColor == 'W' ? 'B' : 'W');
    std::vector<std::string> moves = generateAllLegalMoves(board, color);

    if (moves.empty()) {
        // no legal moves -> evaluate (checkmate/stalemate handled by isCheckmate/isStalemate elsewhere)
        return evaluateBoard(board);
    }

    // Move ordering: prefer captures (bigger captured piece first)
    std::sort(moves.begin(), moves.end(), [&](const std::string &a, const std::string &b) {
        char atarget = board.getSquare(a[2]-'a', '8'-a[3]);
        char btarget = board.getSquare(b[2]-'a', '8'-b[3]);
        double av = (atarget=='.') ? 0.0 : pieceValue(atarget);
        double bv = (btarget=='.') ? 0.0 : pieceValue(btarget);
        return av > bv; // higher capture value first
    });

    double bestVal = maximizing ? -std::numeric_limits<double>::infinity()
                                : std::numeric_limits<double>::infinity();

    for (const std::string &mv : moves) {
        Board copy = board;
        copy.makeMove(mv);
        double val = alphaBeta(copy, depth - 1, alpha, beta, !maximizing);

        // small extra priority if the move is a capture to favor tactical win
        char captured = board.getSquare(mv[2]-'a', '8'-mv[3]);
        if (captured != '.') {
            val += (maximizing ? 1.0 : -1.0) * pieceValue(captured) * 0.25;
        }

        if (maximizing) {
            if (val > bestVal) bestVal = val;
            alpha = std::max(alpha, val);
        } else {
            if (val < bestVal) bestVal = val;
            beta = std::min(beta, val);
        }
        if (beta <= alpha) break; // alpha-beta cut
    }

    // store in TT
    tt[key] = { bestVal, depth };
    return bestVal;
}

// Iterative deepening + alpha-beta search driver
std::string AIPlayer::findBestMove(Board& board) {
    auto t0 = std::chrono::high_resolution_clock::now();

    // clear TT each move (optional) — keeping TT gives cross-depth reuse; we keep it.
    // tt.clear();

    std::vector<std::string> legalMoves = generateAllLegalMoves(board, playerColor);
    if (legalMoves.empty()) return "";

    double baseScore = evaluateBoard(board);
    std::string bestOverall = legalMoves.front();
    double bestOverallScore = -std::numeric_limits<double>::infinity();

    // Iterative deepening
    for (int depth = 1; depth <= maxDepth; ++depth) {
        std::string bestAtDepth = "";
        double bestScoreAtDepth = -std::numeric_limits<double>::infinity();

        // order top-level moves by capture value first
        std::sort(legalMoves.begin(), legalMoves.end(), [&](const std::string &a, const std::string &b) {
            char atarget = board.getSquare(a[2]-'a', '8'-a[3]);
            char btarget = board.getSquare(b[2]-'a', '8'-b[3]);
            double av = (atarget=='.') ? 0.0 : pieceValue(atarget);
            double bv = (btarget=='.') ? 0.0 : pieceValue(btarget);
            return av > bv;
        });

        for (const std::string &mv : legalMoves) {
            Board copy = board;
            copy.makeMove(mv);

            double val = alphaBeta(copy, depth - 1,
                                   -std::numeric_limits<double>::infinity(),
                                    std::numeric_limits<double>::infinity(),
                                   false);

            // extra capture bonus on top-level
            char captured = board.getSquare(mv[2]-'a', '8'-mv[3]);
            if (captured != '.') val += pieceValue(captured) * 0.4;

            // slight randomness / bias to diversify
            char movingPiece = board.getSquare(mv[0]-'a', '8'-mv[1]);
            double bias = 0.0;
            switch (std::toupper(static_cast<unsigned char>(movingPiece))) {
                case 'P': bias = ((std::rand()%100) < 18) ? 0.12 : 0.0; break;
                case 'N': bias = ((std::rand()%100) < 12) ? 0.16 : 0.0; break;
                case 'B': bias = ((std::rand()%100) < 8)  ? 0.16 : 0.0; break;
                case 'R': bias = ((std::rand()%100) < 5)  ? 0.20 : 0.0; break;
                case 'Q': bias = ((std::rand()%100) < 3)  ? 0.25 : 0.0; break;
                case 'K': bias = -0.9; break;
            }
            val += bias;

            if (val > bestScoreAtDepth) {
                bestScoreAtDepth = val;
                bestAtDepth = mv;
            }
        }

        if (!bestAtDepth.empty()) {
            bestOverall = bestAtDepth;
            bestOverallScore = bestScoreAtDepth;
        }

        // small console feedback per depth
        std::cout << "[ID] depth=" << depth << " best=" << bestAtDepth
                  << " score=" << std::fixed << std::setprecision(2) << bestScoreAtDepth << "\n";
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t1 - t0;

    totalThinkingTime += elapsed.count();
    movesCount++;

    // update lastMoveFrom for repetition avoidance
    if (!bestOverall.empty()) lastMoveFrom = bestOverall.substr(0,2);

    // Debug output (kept concise so board display doesn't drown it)
    std::cout << "\n========== AI DEBUG INFO ==========\n";
    std::cout << "AI Colour: " << (playerColor=='W' ? "White" : "Black") << "\n";
    std::cout << "Base Score: " << std::fixed << std::setprecision(2) << baseScore << "\n";
    std::cout << "Chosen Move: " << bestOverall << "   (depth " << maxDepth << ")\n";
    std::cout << "Eval (post-search): " << std::fixed << std::setprecision(2) << bestOverallScore << "\n";
    std::cout << "Thinking Time: " << (elapsed.count()*1000.0) << " ms\n";
    std::cout << "Average Time: " << ((movesCount>0) ? (totalThinkingTime/movesCount*1000.0) : 0.0) << " ms\n";
    std::cout << "===================================\n\n";

    return bestOverall;
}
