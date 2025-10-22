#include "Board.hpp"
#include "AIPlayer.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main() {
    Board board;
    AIPlayer aiWhite('W');
    AIPlayer aiBlack('B');

    std::cout << "Select mode:\n";
    std::cout << "1. Player vs Player\n";
    std::cout << "2. Player (White) vs AI (Black)\n";
    std::cout << "3. AI vs AI\n";
    std::cout << "Enter choice: ";

    int mode;
    std::cin >> mode;

    int moveDelayMs = 800; // default delay between AI moves
    if (mode == 3) {
        std::cout << "Enter delay between AI moves (ms): ";
        std::cin >> moveDelayMs;
    }

    int moveCount = 0;
    board.display();

    while (true) {
        char current = board.getCurrentPlayer();
        std::string move;

        std::cout << "\n--- Move " << (++moveCount) << " ---\n";
        std::cout << "Current player: " << (current == 'W' ? "White" : "Black") << "\n";

        if (mode == 1) {
            std::cout << ((current == 'W') ? "White" : "Black") << " move: ";
            std::cin >> move;
        } 
        else if (mode == 2) {
            if (current == 'W') {
                std::cout << "Enter move (e.g. e2e4) or 'q' to quit: ";
                std::cin >> move;
                if (move == "q") break;
            } else {
                move = aiBlack.findBestMove(board);
                if (move.empty()) {
                    std::cout << "AI has no legal moves.\n";
                    break;
                }
                std::cout << "AI plays: " << move << "\n";
            }
        } 
        else if (mode == 3) {
            move = (current == 'W') ? aiWhite.findBestMove(board)
                                    : aiBlack.findBestMove(board);

            if (move.empty()) {
                std::cout << "AI (" << current << ") has no legal moves.\n";
                break;
            }

            std::cout << "AI (" << current << ") plays: " << move << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(moveDelayMs));
        }

        if (!board.makeMove(move)) continue;
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

    std::cout << "\nGame over after " << moveCount << " moves.\n";
    return 0;
}
