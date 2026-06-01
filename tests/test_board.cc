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
  Board b;                                                     // white to move
  CHECK_FALSE(b.IsLegal(Move{{6, 4}, {4, 4}, std::nullopt}));  // black pawn
}

TEST_CASE("IsLegal rejects illegal knight destination") {
  Board b;
  CHECK_FALSE(b.IsLegal(Move{{0, 1}, {3, 3}, std::nullopt}));
}

TEST_CASE("undo restores starting position") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});
  b.Undo();
  CHECK(b.At(1, 4) != nullptr);
  CHECK(b.At(3, 4) == nullptr);
  CHECK(b.ToMove() == Color::kWhite);
}

TEST_CASE("en passant capture") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});  // e2-e4
  b.Apply(Move{{6, 0}, {5, 0}, std::nullopt});  // a7-a6
  b.Apply(Move{{3, 4}, {4, 4}, std::nullopt});  // e4-e5
  b.Apply(Move{{6, 3}, {4, 3}, std::nullopt});  // d7-d5 (double push)
  // Now white pawn on e5 can EP-capture to d6.
  CHECK(b.IsLegal(Move{{4, 4}, {5, 3}, std::nullopt}));
  b.Apply(Move{{4, 4}, {5, 3}, std::nullopt});
  CHECK(b.At(5, 3) != nullptr);  // pawn on d6
  CHECK(b.At(4, 3) == nullptr);  // captured pawn gone
}
