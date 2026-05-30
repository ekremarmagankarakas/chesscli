#include "rook.h"

#include <vector>

#include "../piece.h"

Rook::Rook(Color color) : Piece(color, PieceType::kRook) {}
char Rook::GetSymbol() const { return GetColor() == Color::kWhite ? 'R' : 'r'; }

std::vector<Square> Rook::ValidMoves(const Square& from,
                                     const Board& board) const {
  std::vector<Square> moves;
  // Directions: up-right, up-left, down-right, down-left
  const std::vector<std::pair<int, int>> directions = {
      {0, 1}, {1, 0}, {0, -1}, {-1, 0}};

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
