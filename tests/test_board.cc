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

TEST_CASE("multiple undos walk back through history") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});  // e2-e4
  b.Apply(Move{{6, 4}, {4, 4}, std::nullopt});  // e7-e5
  b.Undo();
  CHECK(b.At(4, 4) == nullptr);
  CHECK(b.At(6, 4) != nullptr);
  CHECK(b.ToMove() == Color::kBlack);
  b.Undo();
  CHECK(b.At(3, 4) == nullptr);
  CHECK(b.At(1, 4) != nullptr);
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

TEST_CASE("50 Move rule triggers draw") {
  Board b;
  // Shuffle knights back and forth 50 times each side.
  for (int i = 0; i < 25; ++i) {
    b.Apply(Move{{0, 1}, {2, 2}, std::nullopt});  // Nb1-c3
    b.Apply(Move{{7, 1}, {5, 2}, std::nullopt});  // Nb8-c6
    b.Apply(Move{{2, 2}, {0, 1}, std::nullopt});  // Nc3-b1
    b.Apply(Move{{5, 2}, {7, 1}, std::nullopt});  // Nc6-b8
  }
  // 100 knight plies, no pawn/capture.
  CHECK(b.Result() == GameResult::kFiftyMoveDraw);
}

TEST_CASE("threefold repetition triggers draw") {
  Board b;
  for (int i = 0; i < 2; ++i) {
    b.Apply(Move{{0, 1}, {2, 2}, std::nullopt});
    b.Apply(Move{{7, 1}, {5, 2}, std::nullopt});
    b.Apply(Move{{2, 2}, {0, 1}, std::nullopt});
    b.Apply(Move{{5, 2}, {7, 1}, std::nullopt});
  }
  CHECK(b.Result() == GameResult::kThreefoldDraw);
}
