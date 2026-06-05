#include "queen.h"

#include <vector>

#include "../piece.h"
#include "../square.h"

Queen::Queen(Color color) : Piece(color, PieceType::kQueen) {}
char Queen::GetSymbol() const {
  return GetColor() == Color::kWhite ? 'Q' : 'q';
}

std::unique_ptr<Piece> Queen::Clone() const {
  return std::make_unique<Queen>(GetColor());
}

std::vector<Square> Queen::ValidMoves(const Square& from,
                                      const Board& board) const {
  std::vector<Square> moves;
  // All 8 directions: 4 diagonals + 4 orthogonals.
  const std::vector<std::pair<int, int>> directions = {
      {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}};

  for (const auto& [dr, dc] : directions) {
    int r = from.row + dr;
    int c = from.col + dc;
    while (Board::InBounds(r, c)) {
      auto piece = board.At(r, c);
      if (!piece) {
        moves.push_back({r, c});
      } else {
        if (piece->GetColor() != GetColor()) {
          moves.push_back({r, c});  // Can capture
        }
        break;  // Blocked by a piece
      }
      r += dr;
      c += dc;
    }
  }

  return moves;
}
