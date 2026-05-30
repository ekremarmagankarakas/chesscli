#pragma once
#include "../board.h"
#include "../piece.h"

class Pawn : public Piece {
 public:
  explicit Pawn(Color color);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
