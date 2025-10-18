import pygame
import sys

# --- Constants ---
WIDTH, HEIGHT = 640, 640
ROWS, COLS = 8, 8
SQUARE_SIZE = WIDTH // COLS

LIGHT = (238, 238, 210)
DARK = (118, 150, 86)
HIGHLIGHT = (186, 202, 68)

# Unicode chess symbols (for now, these are fine for testing)
PIECES = {
    "r": "♜", "n": "♞", "b": "♝", "q": "♛", "k": "♚", "p": "♟",
    "R": "♖", "N": "♘", "B": "♗", "Q": "♕", "K": "♔", "P": "♙"
}

# --- Starting board state (simple 2D list) ---
START_POS = [
    list("rnbqkbnr"),
    list("pppppppp"),
    list("........"),
    list("........"),
    list("........"),
    list("........"),
    list("PPPPPPPP"),
    list("RNBQKBNR"),
]


class ChessGUI:
    def __init__(self):
        pygame.init()
        self.screen = pygame.display.set_mode((WIDTH, HEIGHT))
        pygame.display.set_caption("Chess GUI")
        self.font = pygame.font.SysFont("DejaVu Sans", 48)
        self.board = [row[:] for row in START_POS]
        self.selected = None

    def draw_board(self):
        """Draw chessboard with alternating colours."""
        for row in range(ROWS):
            for col in range(COLS):
                colour = LIGHT if (row + col) % 2 == 0 else DARK
                if self.selected == (row, col):
                    colour = HIGHLIGHT
                pygame.draw.rect(
                    self.screen, colour,
                    (col * SQUARE_SIZE, row * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE)
                )

    def draw_pieces(self):
        """Draw the chess pieces on top of the board."""
        for row in range(ROWS):
            for col in range(COLS):
                piece = self.board[row][col]
                if piece != ".":
                    text = self.font.render(PIECES[piece], True, (0, 0, 0))
                    text_rect = text.get_rect(center=(
                        col * SQUARE_SIZE + SQUARE_SIZE // 2,
                        row * SQUARE_SIZE + SQUARE_SIZE // 2
                    ))
                    self.screen.blit(text, text_rect)

    def handle_click(self, pos):
        """Handle user clicks on squares."""
        col = pos[0] // SQUARE_SIZE
        row = pos[1] // SQUARE_SIZE
        if self.selected == (row, col):
            self.selected = None
        else:
            self.selected = (row, col)

    def mainloop(self):
        """Main event loop."""
        while True:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    sys.exit()
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    self.handle_click(event.pos)

            self.draw_board()
            self.draw_pieces()
            pygame.display.flip()


if __name__ == "__main__":
    ChessGUI().mainloop()
