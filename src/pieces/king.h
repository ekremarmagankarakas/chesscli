#pragma once
#include "../board.h"
#include "../piece.h"

class King : public Piece {
 public:
  explicit King(Color color);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
