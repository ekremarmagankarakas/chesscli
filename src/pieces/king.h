#pragma once
#include "../board.h"
#include "../piece.h"
#include "../square.h"

namespace chess {

class King final : public Piece {
 public:
  explicit King(Color color);

  std::unique_ptr<Piece> Clone() const override;

  char GetSymbol() const override;
  std::vector<Square> ValidMoves(const Square& from,
                                 const Board& board) const override;
  std::vector<Square> Attacks(const Square& from,
                              const Board& board) const override;
};

}  // namespace chess
