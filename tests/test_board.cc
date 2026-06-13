#include <doctest/doctest.h>

#include "board.h"

using namespace chess;

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

TEST_CASE("resign triggers other side to win") {
  Board b;
  b.Apply(Move{{0, 1}, {2, 2}, std::nullopt});
  b.Apply(Move{{7, 1}, {5, 2}, std::nullopt});
  CHECK(b.HandleResign() == GameResult::kBlackWins);
}

TEST_CASE("Reset restores starting position and clears history") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});
  b.Apply(Move{{6, 4}, {4, 4}, std::nullopt});
  b.Reset();
  CHECK(b.At(1, 4) != nullptr);
  CHECK(b.At(3, 4) == nullptr);
  CHECK(b.At(6, 4) != nullptr);
  CHECK(b.At(4, 4) == nullptr);
  CHECK(b.ToMove() == Color::kWhite);
  // History cleared — Undo should be a no-op.
  b.Undo();
  CHECK(b.At(1, 4) != nullptr);
}

TEST_CASE("insufficient material: K vs K") {
  Board b;
  // Bypass setup by clearing then placing kings only.
  BoardState s{};
  s.side = 0;
  s.castling = 0;
  s.ep_file = -1;
  s.halfmove_clock = 0;
  s.squares[0 * 8 + 4] = 6;   // white king e1
  s.squares[7 * 8 + 4] = 12;  // black king e8
  b.Restore(s);
  CHECK(b.IsInsufficientMaterial());
  CHECK(b.Result() == GameResult::kInsufficientMaterialDraw);
}

TEST_CASE("insufficient material: K+N vs K") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 4] = 6;   // K
  s.squares[7 * 8 + 4] = 12;  // k
  s.squares[0 * 8 + 1] = 2;   // N
  b.Restore(s);
  CHECK(b.IsInsufficientMaterial());
}

TEST_CASE("sufficient material: K+R vs K") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 4] = 6;
  s.squares[7 * 8 + 4] = 12;
  s.squares[0 * 8 + 0] = 4;  // R
  b.Restore(s);
  CHECK_FALSE(b.IsInsufficientMaterial());
}

TEST_CASE("insufficient material: K+B vs K+B same color squares") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 4] = 6;
  s.squares[7 * 8 + 4] = 12;
  s.squares[0 * 8 + 2] = 3;  // White bishop on c1 (dark: 0+2=2)
  s.squares[7 * 8 + 5] = 9;  // Black bishop on f8 (dark: 7+5=12)
  b.Restore(s);
  CHECK(b.IsInsufficientMaterial());
}

TEST_CASE("sufficient material: K+B vs K+B opposite color squares") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 4] = 6;
  s.squares[7 * 8 + 4] = 12;
  s.squares[0 * 8 + 2] = 3;  // White bishop on c1 (dark)
  s.squares[7 * 8 + 2] = 9;  // Black bishop on c8 (light: 7+2=9)
  b.Restore(s);
  CHECK_FALSE(b.IsInsufficientMaterial());
}
