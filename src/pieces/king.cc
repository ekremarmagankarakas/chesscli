#include "king.h"

#include <vector>

#include "../board.h"
#include "../piece.h"
#include "../square.h"

namespace chess {

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

  // Castling targets — fully validated by Board::CanCastle*.
  int home_row = GetColor() == Color::kWhite ? 0 : 7;
  if (from.row == home_row && from.col == 4) {
    if (board.CanCastleKingside(GetColor())) {
      moves.push_back({home_row, 6});
    }
    if (board.CanCastleQueenside(GetColor())) {
      moves.push_back({home_row, 2});
    }
  }

  return moves;
}

std::vector<Square> King::Attacks(const Square& from, const Board&) const {
  std::vector<Square> attacks;
  const std::pair<int, int> dirs[8] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1},
                                       {0, 1}, {1, 0},  {0, -1}, {-1, 0}};
  for (auto [dr, dc] : dirs) {
    int r = from.row + dr;
    int c = from.col + dc;
    if (Board::InBounds(r, c)) {
      attacks.push_back({r, c});
    }
  }
  return attacks;
}

}  // namespace chess
