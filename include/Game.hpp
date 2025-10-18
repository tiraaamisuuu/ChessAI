#pragma once
#include "Board.hpp"
#include "AIPlayer.hpp"

class Game {
public:
    Game();
    void play();

private:
    Board board;
    AIPlayer ai;
};
