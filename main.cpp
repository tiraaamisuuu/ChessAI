#include "Board.hpp"
#include "AIPlayer.hpp"
#include <iostream>
#include <string>

int main() {
    Board board;
    AIPlayer ai('B'); // black AI

    std::string move;
    board.display();

    while (true) {
        // Player move
        if (board.getCurrentPlayer() == 'W') {
            std::cout << "Enter move (e.g. e2e4) or 'q' to quit: ";
            std::cin >> move;
            if (move == "q") break;
            if (!board.makeMove(move)) continue;
        } 
        // AI move
        else {
            std::string aiMove = ai.findBestMove(board);
            if (aiMove.empty()) {
                std::cout << "AI has no legal moves.\n";
                break;
            }
            std::cout << "AI plays: " << aiMove << "\n";
            board.makeMove(aiMove);
        }

        board.display();

        char nextPlayer = board.getCurrentPlayer();
        if (board.isCheckmate(nextPlayer)) {
            std::cout << "Checkmate! " 
                      << (nextPlayer == 'W' ? "Black" : "White") 
                      << " wins!\n";
            break;
        }

        if (board.isStalemate(nextPlayer)) {
            std::cout << "Stalemate! It's a draw.\n";
            break;
        }

        if (board.isInCheck(nextPlayer)) {
            std::cout << (nextPlayer == 'W' ? "White" : "Black")
                      << " is in check!\n";
        }
    }

    std::cout << "Game over.\n";
    return 0;
}
