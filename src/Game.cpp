#include "Game.hpp"
#include <iostream>

Game::Game() : ai(3) {} // depth = 3

void Game::play() {
    board.display();
    std::string move;
    while (true) {
        std::cout << "Enter move (e.g., e2e4): ";
        std::cin >> move;
        board.makeMove(move);
        board.display();
        std::string aiMove = ai.findBestMove(board);
        std::cout << "AI plays: " << aiMove << std::endl;
        board.makeMove(aiMove);
        board.display();
    }
}


