#pragma once
#include "../board.h"
#include "../piece.h"

class Rook : public Piece {
 public:
  explicit Rook(Color color);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
