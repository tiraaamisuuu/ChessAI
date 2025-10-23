#include "Board.hpp"
#include "AIPlayer.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cctype>
#include <vector>
#include <fstream>

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

    int moveDelayMs = 800; // default AI delay
    if (mode == 3) {
        std::cout << "Enter delay between AI moves (ms): ";
        std::cin >> moveDelayMs;
    }

    int moveCount = 0;
    const int maxMoves = 200; // Hard limit
    std::vector<std::string> moveHistory;

    board.display();
    std::cout << "Press 'q' then Enter at any time to quit.\n";

    while (moveCount < maxMoves) {
        char current = board.getCurrentPlayer();
        std::string move;

        std::cout << "\n--- Move " << (moveCount + 1) << " ---\n";
        std::cout << "Current player: " << (current == 'W' ? "White" : "Black") << "\n";

        // Human quit detection (if input is ready)
        if (std::cin.rdbuf()->in_avail() > 0) {
            char c;
            std::cin >> c;
            if (c == 'q' || c == 'Q') {
                std::cout << "Quitting game...\n";
                break;
            }
        }

        // Determine move based on mode
        if (mode == 1) { // PvP
            std::cout << ((current == 'W') ? "White" : "Black") << " move: ";
            std::cin >> move;
            if (move == "q" || move == "Q") break;
        }
        else if (mode == 2) { // PvAI
            if (current == 'W') {
                std::cout << "Enter move (e.g. e2e4) or 'q' to quit: ";
                std::cin >> move;
                if (move == "q" || move == "Q") break;
            } else {
                move = aiBlack.findBestMove(board);
                if (move.empty()) {
                    std::cout << "AI has no legal moves.\n";
                    break;
                }
                std::cout << "AI plays: " << move << "\n";
            }
        }
        else if (mode == 3) { // AIvAI
            move = (current == 'W') ? aiWhite.findBestMove(board)
                                    : aiBlack.findBestMove(board);
            if (move.empty()) {
                std::cout << "AI (" << current << ") has no legal moves.\n";
                break;
            }
            std::cout << "AI (" << current << ") plays: " << move << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(moveDelayMs));
        }

        // Make move
        if (!board.makeMove(move)) {
            std::cout << "Invalid move, try again.\n";
            continue; // Don't increment moveCount if move fails
        }

        moveHistory.push_back(move);
        moveCount++;

        board.display();

        // Check endgame
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

    if (moveCount >= maxMoves) {
        std::cout << "\nReached hard move limit of " << maxMoves << " — draw declared.\n";
    }

    // Write move history to file (overwrite each time)
    std::ofstream outFile("moves.txt");
    for (const auto &m : moveHistory) outFile << m << "\n";
    outFile.close();

    std::cout << "\nGame over after " << moveCount << " moves.\n";
    std::cout << "Move history saved to moves.txt\n";
    return 0;
}
