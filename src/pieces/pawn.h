#pragma once
#include "../board.h"
#include "../piece.h"
#include "../square.h"

class Pawn : public Piece {
 public:
  explicit Pawn(Color color);

  std::unique_ptr<Piece> Clone() const override;

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};
