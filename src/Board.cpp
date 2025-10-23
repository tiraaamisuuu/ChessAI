#include "Board.hpp"
#include "AIPlayer.hpp"
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <iostream>

// Constructor: set up initial chessboard, current player, and last move
Board::Board() : currentPlayer('W'), lastMove(""), enPassantX(-1), enPassantY(-1) {
    const std::string initial[8] = {
        "rnbqkbnr", // 8th rank (black)
        "pppppppp", // 7th rank
        "........", // 6th rank
        "........", // 5th rank
        "........", // 4th rank
        "........", // 3rd rank
        "PPPPPPPP", // 2nd rank (white)
        "RNBQKBNR"  // 1st rank (white back rank)
    };

    for (int y = 0; y < 8; ++y)
        for (int x = 0; x < 8; ++x)
            squares[y][x] = initial[y][x];
}

// Display board (white bottom). Uses Unicode chess glyphs for readability.
void Board::display() const {
    std::cout << "\033[2J\033[1;1H"; // clear screen

    std::cout << "  a b c d e f g h\n";
    for (int y = 0; y < 8; ++y) {
        std::cout << (8 - y) << " ";
        for (int x = 0; x < 8; ++x) {
            char piece = squares[y][x];
            switch (piece) {
                case 'P': std::cout << "♙ "; break;
                case 'p': std::cout << "♟ "; break;
                case 'R': std::cout << "♖ "; break;
                case 'r': std::cout << "♜ "; break;
                case 'N': std::cout << "♘ "; break;
                case 'n': std::cout << "♞ "; break;
                case 'B': std::cout << "♗ "; break;
                case 'b': std::cout << "♝ "; break;
                case 'Q': std::cout << "♕ "; break;
                case 'q': std::cout << "♛ "; break;
                case 'K': std::cout << "♔ "; break;
                case 'k': std::cout << "♚ "; break;
                case '.': std::cout << ". "; break;
                default:  std::cout << "? "; break;
            }
        }
        std::cout << (8 - y) << "\n";
    }
    std::cout << "  a b c d e f g h\n\n";

    std::cout << "Current player: " << (currentPlayer == 'W' ? "White" : "Black") << "\n";
    if (!lastMove.empty()) std::cout << "Last move: " << lastMove << "\n";

    // --- Score Bar (Evaluation Display) ---
    AIPlayer evaluator('W'); // evaluate from White's perspective
    double score = evaluator.evaluateBoard(*this);

    // Cap and scale score for display
    if (score > 10) score = 10;
    if (score < -10) score = -10;

    int barWidth = 30;
    int whiteBlocks = static_cast<int>((score + 10) / 20 * barWidth);

    std::cout << "Eval: ";
    for (int i = 0; i < barWidth; ++i) {
        if (i < whiteBlocks)
            std::cout << "█"; // white advantage
        else
            std::cout << "░"; // black advantage
    }
    std::cout << "  (" << (score >= 0 ? "+" : "") << score << ")\n\n";
}
// file 'a'..'h' -> 0..7
int Board::fileToX(char file) const { return file - 'a'; }

// rank '1'..'8' -> array row 7..0 (so '1' -> 7, '8' -> 0)
int Board::rankToY(char rank) const { return '8' - rank; }

// path check for sliding pieces
bool Board::isPathClear(int fromX, int fromY, int toX, int toY) const {
    int dx = (toX > fromX) ? 1 : (toX < fromX ? -1 : 0);
    int dy = (toY > fromY) ? 1 : (toY < fromY ? -1 : 0);

    int x = fromX + dx;
    int y = fromY + dy;
    while (x != toX || y != toY) {
        if (squares[y][x] != '.') return false;
        x += dx; y += dy;
    }
    return true;
}

// Pawn movement (includes regular capture + two-step start; en-passant handled in validate)
bool Board::isValidPawnMove(int fromX, int fromY, int toX, int toY) const {
    char piece = squares[fromY][fromX];
    int dir = std::isupper(piece) ? -1 : 1;
    int startRow = std::isupper(piece) ? 6 : 1;

    // 1-step
    if (fromX == toX && toY - fromY == dir && squares[toY][toX] == '.') return true;

    // 2-step
    if (fromX == toX && fromY == startRow && toY - fromY == 2*dir &&
        squares[fromY + dir][fromX] == '.' && squares[toY][toX] == '.') return true;

    // capture (normal)
    if (std::abs(toX - fromX) == 1 && toY - fromY == dir &&
        squares[toY][toX] != '.' &&
        ((std::isupper(piece) && std::islower(squares[toY][toX])) ||
         (std::islower(piece) && std::isupper(squares[toY][toX]))))
        return true;

    // en-passant capture will be validated in validateMove (needs context of enPassantX/Y)
    return false;
}

bool Board::isValidRookMove(int fromX, int fromY, int toX, int toY) const {
    if (fromX != toX && fromY != toY) return false;
    return isPathClear(fromX, fromY, toX, toY);
}

bool Board::isValidBishopMove(int fromX, int fromY, int toX, int toY) const {
    if (std::abs(toX - fromX) != std::abs(toY - fromY)) return false;
    return isPathClear(fromX, fromY, toX, toY);
}

bool Board::isValidKnightMove(int fromX, int fromY, int toX, int toY) const {
    int dx = std::abs(toX - fromX), dy = std::abs(toY - fromY);
    return (dx == 1 && dy == 2) || (dx == 2 && dy == 1);
}

bool Board::isValidQueenMove(int fromX, int fromY, int toX, int toY) const {
    return isValidRookMove(fromX, fromY, toX, toY) || isValidBishopMove(fromX, fromY, toX, toY);
}

bool Board::isValidKingMove(int fromX, int fromY, int toX, int toY) const {
    int dx = toX - fromX;
    int dy = toY - fromY;

    // Normal king move
    if (std::max(std::abs(dx), std::abs(dy)) == 1)
        return true;

    // Castling
    if (dy == 0 && std::abs(dx) == 2) {
        if (currentPlayer == 'W') {
            if (whiteKingMoved) return false;

            // Kingside
            if (dx == 2 && !whiteRookMoved[1] &&
                squares[7][5] == '.' && squares[7][6] == '.' &&
                !isSquareAttacked(4,7,false) && !isSquareAttacked(5,7,false) && !isSquareAttacked(6,7,false))
                return true;

            // Queenside
            if (dx == -2 && !whiteRookMoved[0] &&
                squares[7][1] == '.' && squares[7][2] == '.' && squares[7][3] == '.' &&
                !isSquareAttacked(4,7,false) && !isSquareAttacked(3,7,false) && !isSquareAttacked(2,7,false))
                return true;
        } else {
            if (blackKingMoved) return false;

            // Kingside
            if (dx == 2 && !blackRookMoved[1] &&
                squares[0][5] == '.' && squares[0][6] == '.' &&
                !isSquareAttacked(4,0,true) && !isSquareAttacked(5,0,true) && !isSquareAttacked(6,0,true))
                return true;

            // Queenside
            if (dx == -2 && !blackRookMoved[0] &&
                squares[0][1] == '.' && squares[0][2] == '.' && squares[0][3] == '.' &&
                !isSquareAttacked(4,0,true) && !isSquareAttacked(3,0,true) && !isSquareAttacked(2,0,true))
                return true;
        }
    }

    return false;
}

bool Board::isCorrectPlayerMove(char piece) const {
    return (currentPlayer == 'W') ? std::isupper(piece) : std::islower(piece);
}

// Detailed validation with error messages
std::string Board::validateMove(const std::string &move) const {
    if (move.length() != 4) return "Move must be 4 characters (e.g. e2e4).";

    int fromX = fileToX(move[0]);
    int fromY = rankToY(move[1]);
    int toX   = fileToX(move[2]);
    int toY   = rankToY(move[3]);

    if (fromX < 0 || fromX > 7 || toX < 0 || toX > 7 ||
        fromY < 0 || fromY > 7 || toY < 0 || toY > 7)
        return "Move is out of bounds.";

    char piece = squares[fromY][fromX];
    if (piece == '.') return "No piece at source square.";

    if (!isCorrectPlayerMove(piece)) return "That piece does not belong to you.";

    char dest = squares[toY][toX];
    if (dest != '.' &&
        ((std::isupper(piece) && std::isupper(dest)) || (std::islower(piece) && std::islower(dest))))
        return "Cannot capture your own piece.";

    bool valid = false;
    switch (std::toupper(piece)) {
        case 'P':
            // standard pawn movement & captures
            valid = isValidPawnMove(fromX, fromY, toX, toY);
            // EN PASSANT: target square must be empty, moving diagonally one, and match enPassantX/Y
            if (!valid) {
                int dx = toX - fromX;
                int dy = toY - fromY;
                int dir = std::isupper(piece) ? -1 : 1;
                if (std::abs(dx) == 1 && dy == dir && dest == '.' &&
                    toX == enPassantX && toY == enPassantY) {
                    // en-passant capture is allowed
                    valid = true;
                }
            }
            break;
        case 'R': valid = isValidRookMove(fromX, fromY, toX, toY); break;
        case 'N': valid = isValidKnightMove(fromX, fromY, toX, toY); break;
        case 'B': valid = isValidBishopMove(fromX, fromY, toX, toY); break;
        case 'Q': valid = isValidQueenMove(fromX, fromY, toX, toY); break;
        case 'K': valid = isValidKingMove(fromX, fromY, toX, toY); break;
    }
    if (!valid) return "That piece cannot move like that.";

    if (wouldLeaveKingInCheck(fromX, fromY, toX, toY))
        return "Move would leave your king in check.";

    return "";
}

// Simulate move and check if it leaves current player in check
bool Board::wouldLeaveKingInCheck(int fromX, int fromY, int toX, int toY) const {
    Board copy = *this;
    char piece = copy.squares[fromY][fromX];

    // Handle castling move simulation same as actual move (move rook too)
    if (std::toupper(piece) == 'K' && std::abs(toX - fromX) == 2) {
        copy.squares[toY][toX] = piece;
        copy.squares[fromY][fromX] = '.';
        if (toX > fromX) {
            // kingside: move rook from h-file
            copy.squares[toY][toX-1] = copy.squares[toY][7];
            copy.squares[toY][7] = '.';
        } else {
            // queenside: move rook from a-file
            copy.squares[toY][toX+1] = copy.squares[toY][0];
            copy.squares[toY][0] = '.';
        }
    }
    // En-passant simulation: if pawn moves to enPassant square, remove captured pawn
    else if (std::toupper(piece) == 'P' && std::abs(toX - fromX) == 1 && squares[toY][toX] == '.' &&
             toX == enPassantX && toY == enPassantY) {
        // move pawn
        copy.squares[toY][toX] = piece;
        copy.squares[fromY][fromX] = '.';
        // remove the pawn that was captured en-passant: it's at (toX, fromY)
        copy.squares[fromY][toX] = '.';
    } else {
        copy.squares[toY][toX] = piece;
        copy.squares[fromY][fromX] = '.';
    }

    return copy.isInCheck(currentPlayer);
}

// Detect if a square is attacked by pieces of the specified colour
// byWhite == true => check attacks by white pieces, false => black pieces
bool Board::isSquareAttacked(int x, int y, bool byWhite) const {
    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = squares[fromY][fromX];
            if (piece == '.') continue;

            bool pieceIsWhite = std::isupper(static_cast<unsigned char>(piece));
            if (pieceIsWhite != byWhite) continue;

            char t = std::toupper(static_cast<unsigned char>(piece));

            // Pawn attacks (pawns attack diagonally)
            if (t == 'P') {
                int dir = pieceIsWhite ? -1 : 1;
                if ((fromX - 1 == x && fromY + dir == y) ||
                    (fromX + 1 == x && fromY + dir == y))
                    return true;
                continue;
            }

            // Knight attacks
            if (t == 'N') {
                static const int kx[] = {1,2,2,1,-1,-2,-2,-1};
                static const int ky[] = {2,1,-1,-2,-2,-1,1,2};
                for (int i = 0; i < 8; ++i)
                    if (fromX + kx[i] == x && fromY + ky[i] == y) return true;
                continue;
            }

            // King attacks
            if (t == 'K') {
                if (std::max(std::abs(fromX - x), std::abs(fromY - y)) == 1) return true;
                continue;
            }

            // Sliding pieces (Rook, Bishop, Queen)
            if (t == 'R' || t == 'Q') {
                if (fromX == x || fromY == y) {
                    if (isPathClear(fromX, fromY, x, y)) return true;
                }
            }
            if (t == 'B' || t == 'Q') {
                if (std::abs(fromX - x) == std::abs(fromY - y)) {
                    if (isPathClear(fromX, fromY, x, y)) return true;
                }
            }
        }
    }
    return false;
}

// New: check if player’s king is in check
// Return true if the given colour is in check
bool Board::isInCheck(char color) const {
    char kingChar = (color == 'W') ? 'K' : 'k';
    int kx = -1, ky = -1;
    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            if (squares[y][x] == kingChar) { kx = x; ky = y; break; }
        }
        if (kx != -1) break;
    }

    if (kx == -1) return true; // King not found: treat as in check

    // Check attacks by opponent
    bool attackedByWhite = (color == 'B'); // if checking black king, check attacks by white
    return isSquareAttacked(kx, ky, attackedByWhite);
}

// New: detect if player has any legal move
bool Board::hasAnyLegalMove(char player) const {
    for (int fromY = 0; fromY < 8; ++fromY) {
        for (int fromX = 0; fromX < 8; ++fromX) {
            char piece = squares[fromY][fromX];
            if (piece == '.') continue;
            if ((player == 'W' && std::isupper(piece)) ||
                (player == 'B' && std::islower(piece))) {
                for (int toY = 0; toY < 8; ++toY) {
                    for (int toX = 0; toX < 8; ++toX) {
                        std::string mv = std::string() + char('a' + fromX) + char('8' - fromY)
                                         + char('a' + toX) + char('8' - toY);
                        if (validateMove(mv).empty()) return true;
                    }
                }
            }
        }
    }
    return false;
}

// ✅ New: checkmate / stalemate
bool Board::isCheckmate(char player) const {
    return isInCheck(player) && !hasAnyLegalMove(player);
}
bool Board::isStalemate(char player) const {
    return !isInCheck(player) && !hasAnyLegalMove(player);
}

// Updated makeMove uses validateMove
bool Board::makeMove(const std::string &move) {
    std::string error = validateMove(move);
    if (!error.empty()) {
        std::cout << "Invalid move: " << error << "\n";
        return false;
    }

    int fromX = fileToX(move[0]);
    int fromY = rankToY(move[1]);
    int toX   = fileToX(move[2]);
    int toY   = rankToY(move[3]);

    char moved = squares[fromY][fromX];

    // Detect castling
    bool isCastling = false;
    if (std::toupper(moved) == 'K' && std::abs(toX - fromX) == 2) {
        isCastling = true;
        // White kingside
        if (currentPlayer == 'W' && toX == 6) {
            squares[7][5] = 'R';
            squares[7][7] = '.';
            whiteKingMoved = true;
            whiteRookMoved[1] = true;
        }
        // White queenside
        else if (currentPlayer == 'W' && toX == 2) {
            squares[7][3] = 'R';
            squares[7][0] = '.';
            whiteKingMoved = true;
            whiteRookMoved[0] = true;
        }
        // Black kingside
        else if (currentPlayer == 'B' && toX == 6) {
            squares[0][5] = 'r';
            squares[0][7] = '.';
            blackKingMoved = true;
            blackRookMoved[1] = true;
        }
        // Black queenside
        else if (currentPlayer == 'B' && toX == 2) {
            squares[0][3] = 'r';
            squares[0][0] = '.';
            blackKingMoved = true;
            blackRookMoved[0] = true;
        }
    }

    // EN PASSANT capture handling: if pawn moves diagonally to empty square and it equals enPassant target
    bool performedEnPassant = false;
    if (std::toupper(moved) == 'P' && std::abs(toX - fromX) == 1 && squares[toY][toX] == '.' &&
        toX == enPassantX && toY == enPassantY) {
        // remove the pawn that did the two-step: it sits at (toX, fromY)
        squares[fromY][toX] = '.';
        performedEnPassant = true;
    }

    // Normal move (also covers promotion below)
    squares[toY][toX] = moved;
    squares[fromY][fromX] = '.';

    // Pawn promotion
    if (std::toupper(moved) == 'P') {
        if ((std::isupper(moved) && toY == 0) || (std::islower(moved) && toY == 7)) {
            squares[toY][toX] = std::isupper(moved) ? 'Q' : 'q';
            std::cout << "Pawn promoted to Queen!\n";
        }
    }

    // Track king and rook moves for future castling
    if (!isCastling) {
        if (moved == 'K') whiteKingMoved = true;
        if (moved == 'k') blackKingMoved = true;
        if (fromX == 0 && fromY == 7 && moved == 'R') whiteRookMoved[0] = true;
        if (fromX == 7 && fromY == 7 && moved == 'R') whiteRookMoved[1] = true;
        if (fromX == 0 && fromY == 0 && moved == 'r') blackRookMoved[0] = true;
        if (fromX == 7 && fromY == 0 && moved == 'r') blackRookMoved[1] = true;
    }

    // EN PASSANT: set or clear enPassant target
    // If this move was a pawn 2-step, set target; otherwise clear it.
    if (std::toupper(moved) == 'P' && std::abs(toY - fromY) == 2) {
        // direction from->to: dir = sign(toY - fromY)
        int dir = (toY - fromY) > 0 ? 1 : -1;
        enPassantX = toX;             // file of pawn that jumped
        enPassantY = fromY + dir;     // square passed over
    } else {
        // clear en-passant when any other move occurs (including en-passant captures)
        enPassantX = -1;
        enPassantY = -1;
    }

    lastMove = move;
    currentPlayer = (currentPlayer == 'W') ? 'B' : 'W';
    return true;
}
