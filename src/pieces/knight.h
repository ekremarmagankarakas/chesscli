#pragma once
#include "../board.h"
#include "../piece.h"
#include "../square.h"

namespace chess {

class Knight final : public Piece {
 public:
  explicit Knight(Color color);

  std::unique_ptr<Piece> Clone() const override;

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
};

}  // namespace chess
