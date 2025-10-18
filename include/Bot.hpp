#ifndef BOT_HPP
#define BOT_HPP

#include <string>
#include "Board.hpp"

class Bot {
public:
    virtual ~Bot() = default;

    // Given the current board, return a move in "e2e4" format
    virtual std::string getMove(Board &board, char botColor) = 0;
};

#endif
