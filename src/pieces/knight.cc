#include "knight.h"

#include <vector>

#include "../piece.h"
#include "../square.h"

Knight::Knight(Color color) : Piece(color, PieceType::kKnight) {}
char Knight::GetSymbol() const {
  return GetColor() == Color::kWhite ? 'N' : 'n';
}

std::unique_ptr<Piece> Knight::Clone() const {
  return std::make_unique<Knight>(GetColor());
}

std::vector<Square> Knight::ValidMoves(const Square& from,
                                       const Board& board) const {
  std::vector<Square> moves;
  // Knight moves: 8 possible L-shaped moves
  const std::vector<std::pair<int, int>> knight_moves = {
      {2, 1}, {2, -1}, {-2, 1}, {-2, -1}, {1, 2}, {1, -2}, {-1, 2}, {-1, -2}};
  for (const auto& [dr, dc] : knight_moves) {
    int r = from.row + dr;
    int c = from.col + dc;
    if (Board::InBounds(r, c)) {
      auto piece = board.At(r, c);
      if (!piece || piece->GetColor() != GetColor()) {
        moves.push_back({r, c});  // Can move or capture
      }
    }
  }
  return moves;
}
