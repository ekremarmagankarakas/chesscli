#pragma once
#include "../board.h"
#include "../piece.h"

class Bishop : public Piece {
 public:
  Bishop(Color color, PieceType type);

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
