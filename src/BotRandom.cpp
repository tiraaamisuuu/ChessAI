#include "Board.hpp"
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

class BotRandom {
public:
    BotRandom() { std::srand(std::time(nullptr)); }

    std::string getMove(Board &board) {
        std::vector<std::string> legalMoves = generateAllLegalMoves(board);
        if (legalMoves.empty()) return "";

        int index = std::rand() % legalMoves.size();
        return legalMoves[index];
    }

private:
    std::vector<std::string> generateAllLegalMoves(Board &board) {
        std::vector<std::string> moves;
        char player = board.getCurrentPlayer();

        for (int fromY = 0; fromY < 8; ++fromY) {
            for (int fromX = 0; fromX < 8; ++fromX) {
                char piece = board.getSquare(fromX, fromY);
                if (piece == '.') continue;

                if ((player == 'W' && std::isupper(piece)) ||
                    (player == 'B' && std::islower(piece))) {
                    for (int toY = 0; toY < 8; ++toY) {
                        for (int toX = 0; toX < 8; ++toX) {
                            std::string mv;
                            mv += char('a' + fromX);
                            mv += char('8' - fromY);
                            mv += char('a' + toX);
                            mv += char('8' - toY);

                            if (board.isMoveValid(mv)) {
                                moves.push_back(mv);
                            }
                        }
                    }
                }
            }
        }
        return moves;
    }
};
