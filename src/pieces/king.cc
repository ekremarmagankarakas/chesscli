#include "king.h"

#include <vector>

#include "../piece.h"
#include "../square.h"

King::King(Color color) : Piece(color, PieceType::kKing) {}
char King::GetSymbol() const { return GetColor() == Color::kWhite ? 'K' : 'k'; }

std::unique_ptr<Piece> King::Clone() const {
  return std::make_unique<King>(GetColor());
}

std::vector<Square> King::ValidMoves(const Square& from,
                                     const Board& board) const {
  std::vector<Square> moves;
  // Directions: up-right, up-left, down-right, down-left, up, right, left, down
  const std::vector<std::pair<int, int>> directions = {
      {1, 1}, {1, -1}, {-1, 1}, {-1, -1}, {0, 1}, {1, 0}, {0, -1}, {-1, 0}};

  for (const auto& [dr, dc] : directions) {
    int r = from.row + dr;
    int c = from.col + dc;
    if (!Board::InBounds(r, c)) {
      continue;
    }
    auto piece = board.At(r, c);
    if (!piece || piece->GetColor() != GetColor()) {
      moves.push_back({r, c});
    }
  }

  return moves;
}
