#include "king.h"

#include <vector>

#include "../piece.h"

King::King(Color color, PieceType type) : Piece(color, type) {}
char King::GetSymbol() const { return GetColor() == Color::kWhite ? 'K' : 'k'; }

std::vector<Square> King::ValidMoves(const Square& from,
                                     const Board& board) const {
  std::vector<Square> moves;
  // Directions: up-right, up-left, down-right, down-left, up, right, left, down
  const std::vector<std::pair<int, int>> directions = {
      {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}};

  for (const auto& [dr, dc] : directions) {
    int r = from.row + dr;
    int c = from.col + dc;
    auto piece = board.At(r, c);
    if (!piece) {
      moves.push_back({r, c});
    } else {
      if (piece->GetColor() != GetColor()) {
        moves.push_back({r, c});  // Can capture
      }
    }
  }

  return moves;
}
