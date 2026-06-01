#include <doctest/doctest.h>

#include <algorithm>

#include "board.h"

TEST_CASE("knight on b1 has 2 starting moves") {
  Board b;
  auto moves = b.At(0, 1)->ValidMoves({0, 1}, b);
  CHECK(moves.size() == 2);
  CHECK(std::find(moves.begin(), moves.end(), Square{2, 0}) != moves.end());
  CHECK(std::find(moves.begin(), moves.end(), Square{2, 2}) != moves.end());
}

TEST_CASE("pawn on e2 can push 1 or 2") {
  Board b;
  auto moves = b.At(1, 4)->ValidMoves({1, 4}, b);
  CHECK(moves.size() == 2);
}

TEST_CASE("bishop on c1 is blocked at start") {
  Board b;
  auto moves = b.At(0, 2)->ValidMoves({0, 2}, b);
  CHECK(moves.empty());
}
