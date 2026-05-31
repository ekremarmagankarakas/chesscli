#pragma once

#include "../board.h"
#include "../piece.h"
#include "../square.h"

class Rook : public Piece {
 public:
  explicit Rook(Color color);

  std::unique_ptr<Piece> Clone() const override;

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
