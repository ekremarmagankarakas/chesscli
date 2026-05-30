#pragma once
#include "../board.h"
#include "../piece.h"

class Knight : public Piece {
 public:
  explicit Knight(Color color);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
