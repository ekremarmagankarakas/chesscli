#include "bishop.h"

#include <vector>

#include "../piece.h"
#include "../square.h"

Bishop::Bishop(Color color) : Piece(color, PieceType::kBishop) {}
char Bishop::GetSymbol() const {
  return GetColor() == Color::kWhite ? 'B' : 'b';
}

std::unique_ptr<Piece> Bishop::Clone() const {
  return std::make_unique<Bishop>(GetColor());
}

std::vector<Square> Bishop::ValidMoves(const Square& from,
                                       const Board& board) const {
  std::vector<Square> moves;
  // Directions: up-right, up-left, down-right, down-left
  const std::vector<std::pair<int, int>> directions = {
      {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

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
