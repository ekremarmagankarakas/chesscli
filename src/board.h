#pragma once

#include <array>
#include <memory>

#include "piece.h"

class Board {
 public:
  static constexpr int kSize = 8;

  Board();

  void Setup();
  void Print() const;

  Piece* At(const Square& square) const;
  Piece* At(int row, int col) const;

  // Moves a piece, overwriting whatever is on the destination square.
  // Does not validate legality. Toggles side to move.
  void MovePiece(const Square& from, const Square& to);

  Color ToMove() const { return side_to_move_; }

  static bool InBounds(const Square& square);
  static bool InBounds(int row, int col);

 private:
  std::array<std::array<std::unique_ptr<Piece>, kSize>, kSize> grid_;
  Color side_to_move_ = Color::kWhite;
};
