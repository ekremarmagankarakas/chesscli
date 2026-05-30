#pragma once

#include <array>
#include <memory>

#include "piece.h"

class Board {
 public:
  static constexpr int kSize = 8;

  Board();  // sets up the starting position

  void Setup();
  void Print() const;

  // Returns nullptr if the square is empty.
  Piece* At(const Square& square) const;
  Piece* At(int row, int col) const;

  // Moves a piece, overwriting whatever is on the destination square.
  // Does not validate legality — that's the caller's job.
  void MovePiece(const Square& from, const Square& to);

  static bool InBounds(const Square& square);
  static bool InBounds(int row, int col);

 private:
  std::array<std::array<std::unique_ptr<Piece>, kSize>, kSize> grid_;
};
