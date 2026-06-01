#include "pawn.h"

#include <vector>

#include "../board.h"
#include "../piece.h"
#include "../square.h"

Pawn::Pawn(Color color) : Piece(color, PieceType::kPawn) {}
char Pawn::GetSymbol() const { return GetColor() == Color::kWhite ? 'P' : 'p'; }

std::unique_ptr<Piece> Pawn::Clone() const {
  return std::make_unique<Pawn>(GetColor());
}

std::vector<Square> Pawn::ValidMoves(const Square& from,
                                     const Board& board) const {
  std::vector<Square> moves;
  const int dir = GetColor() == Color::kWhite ? 1 : -1;
  const int start_row = GetColor() == Color::kWhite ? 1 : 6;

  // Single push.
  int r = from.row + dir;
  int c = from.col;
  if (Board::InBounds(r, c) && !board.At(r, c)) {
    moves.push_back({r, c});

    // Double push from starting rank.
    int r2 = from.row + 2 * dir;
    if (from.row == start_row && Board::InBounds(r2, c) && !board.At(r2, c)) {
      moves.push_back({r2, c});
    }
  }

  // Diagonal captures.
  for (int dc : {-1, 1}) {
    int cr = from.row + dir;
    int cc = from.col + dc;
    if (!Board::InBounds(cr, cc)) {
      continue;
    }
    auto target = board.At(cr, cc);
    if (target && target->GetColor() != GetColor()) {
      moves.push_back({cr, cc});
    }
  }

  return moves;
}

std::vector<Square> Pawn::Attacks(const Square& from, const Board&) const {
  std::vector<Square> attacks;
  const int dir = GetColor() == Color::kWhite ? 1 : -1;
  for (int dc : {-1, 1}) {
    int cr = from.row + dir;
    int cc = from.col + dc;
    if (Board::InBounds(cr, cc)) {
      attacks.push_back({cr, cc});
    }
  }
  return attacks;
}
