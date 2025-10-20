# ChessAI

ChessAI is a C++ chess engine with a simple AI that evaluates board positions and selects moves.  
This project is mainly for learning, experimenting, and testing AI strategies in chess.

## Features

- Full chessboard representation
- Move validation including castling and pawn promotion
- Material-based AI evaluation
- Check, checkmate, and stalemate detection
- AIPlayer class for automated play

## Setup

1. Clone the repository:
```bash
git clone https://github.com/tiraaamisuuu/ChessAI.git
cd ChessAI
rm -rf build
cmake -B build -S .
cmake --build build
./build/ChessAI
```
## How to Play

- Enter moves in standard format (e.g., `e2e4`).
- To castle, move the king two squares (`e1g1` for white kingside, `e1c1` for white queenside, etc.).
- The board will display the current player, last move, and a simple evaluation bar.

## Contributing

Feel free to submit issues or pull requests to improve the AI or add new features.
