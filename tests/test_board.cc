#include <doctest/doctest.h>

#include "board.h"

TEST_CASE("Apply moves piece and flips side") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});
  CHECK(b.At(1, 4) == nullptr);
  CHECK(b.At(3, 4) != nullptr);
  CHECK(b.ToMove() == Color::kBlack);
}

TEST_CASE("IsLegal rejects moving opponent's piece") {
  Board b;  // white to move
  CHECK_FALSE(b.IsLegal(Move{{6, 4}, {4, 4}, std::nullopt}));  // black pawn
}

TEST_CASE("IsLegal rejects illegal knight destination") {
  Board b;
  CHECK_FALSE(b.IsLegal(Move{{0, 1}, {3, 3}, std::nullopt}));
}
