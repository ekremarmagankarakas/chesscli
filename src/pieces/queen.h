#pragma once
#include "../board.h"
#include "../piece.h"

class Queen : public Piece {
 public:
  explicit Queen(Color color);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
