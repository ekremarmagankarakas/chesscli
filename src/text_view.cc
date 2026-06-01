#include "text_view.h"

#include <iostream>

#include "board.h"
#include "piece.h"

void TextView::Render(const Board& board) {
  for (int row = Board::kSize - 1; row >= 0; --row) {
    std::cout << (row + 1) << ' ';
    for (int col = 0; col < Board::kSize; ++col) {
      Piece* piece = board.At(row, col);
      if (piece) {
        std::cout << piece->GetSymbol() << ' ';
      } else {
        std::cout << ". ";
      }
    }
    std::cout << '\n';
  }
  std::cout << "  a b c d e f g h\n";
}

void TextView::ShowMessage(std::string_view msg) { std::cout << msg << '\n'; }

void TextView::ShowGameOver(std::string_view reason) {
  std::cout << reason << '\n';
}
