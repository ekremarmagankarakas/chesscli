#include "parser.h"

#include <doctest/doctest.h>

#include <variant>

#include "board.h"
#include "board_state.h"
#include "command.h"

using namespace chess;

// ---------- ParseUCI (stateless) ----------

TEST_CASE("ParseUCI: e2e4") {
  auto m = ParseUCI("e2e4");
  REQUIRE(m);
  CHECK(m->from == Square{1, 4});
  CHECK(m->to == Square{3, 4});
  CHECK(!m->promotion);
}

TEST_CASE("ParseUCI: promotion e7e8q") {
  auto m = ParseUCI("e7e8q");
  REQUIRE(m);
  CHECK(m->promotion == PieceType::kQueen);
}

TEST_CASE("ParseUCI: rejects garbage") {
  CHECK(!ParseUCI(""));
  CHECK(!ParseUCI("e4"));      // too short
  CHECK(!ParseUCI("Nf3"));     // SAN-shaped
  CHECK(!ParseUCI("e2e4xx"));  // too long
  CHECK(!ParseUCI("z9z9"));    // bad squares
  CHECK(!ParseUCI("e7e8x"));   // bad promotion
}

// ---------- ParseSAN (board-dependent) ----------

TEST_CASE("ParseSAN: pawn push e4") {
  Board b;
  auto m = ParseSAN("e4", b);
  REQUIRE(m);
  CHECK(m->from == Square{1, 4});
  CHECK(m->to == Square{3, 4});
}

TEST_CASE("ParseSAN: knight Nf3") {
  Board b;
  auto m = ParseSAN("Nf3", b);
  REQUIRE(m);
  CHECK(m->from == Square{0, 6});
  CHECK(m->to == Square{2, 5});
}

TEST_CASE("ParseSAN: castling O-O") {
  Board b;
  auto m = ParseSAN("O-O", b);
  REQUIRE(m);
  CHECK(m->from == Square{0, 4});
  CHECK(m->to == Square{0, 6});
}

TEST_CASE("ParseSAN: castling O-O-O") {
  Board b;
  auto m = ParseSAN("O-O-O", b);
  REQUIRE(m);
  CHECK(m->from == Square{0, 4});
  CHECK(m->to == Square{0, 2});
}

TEST_CASE("ParseSAN: pawn capture exd5") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});  // 1. e4
  b.Apply(Move{{6, 3}, {4, 3}, std::nullopt});  // 1... d5
  auto m = ParseSAN("exd5", b);
  REQUIRE(m);
  CHECK(m->from == Square{3, 4});
  CHECK(m->to == Square{4, 3});
}

TEST_CASE("ParseSAN: promotion e8=Q") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 0] = 6;   // wK a1
  s.squares[7 * 8 + 7] = 12;  // bK h8
  s.squares[6 * 8 + 4] = 1;   // wP e7
  b.Restore(s);
  auto m = ParseSAN("e8=Q", b);
  REQUIRE(m);
  CHECK(m->from == Square{6, 4});
  CHECK(m->to == Square{7, 4});
  CHECK(m->promotion == PieceType::kQueen);
}

TEST_CASE("ParseSAN: strips trailing check Nxe5+") {
  Board b;
  b.Apply(Move{{1, 4}, {3, 4}, std::nullopt});
  b.Apply(Move{{6, 4}, {4, 4}, std::nullopt});
  b.Apply(Move{{0, 6}, {2, 5}, std::nullopt});
  b.Apply(Move{{6, 0}, {5, 0}, std::nullopt});
  auto m = ParseSAN("Nxe5+", b);
  REQUIRE(m);
  CHECK(m->from == Square{2, 5});
  CHECK(m->to == Square{4, 4});
}

TEST_CASE("ParseSAN: disambiguation by file Nbd2") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 0] = 6;
  s.squares[7 * 8 + 7] = 12;
  s.squares[0 * 8 + 1] = 2;  // wN b1
  s.squares[2 * 8 + 5] = 2;  // wN f3
  b.Restore(s);
  auto m = ParseSAN("Nbd2", b);
  REQUIRE(m);
  CHECK(m->from == Square{0, 1});
  CHECK(m->to == Square{1, 3});
}

TEST_CASE("ParseSAN: ambiguous Nd2 returns nullopt") {
  Board b;
  BoardState s{};
  s.ep_file = -1;
  s.squares[0 * 8 + 0] = 6;
  s.squares[7 * 8 + 7] = 12;
  s.squares[0 * 8 + 1] = 2;
  s.squares[2 * 8 + 5] = 2;
  b.Restore(s);
  CHECK(!ParseSAN("Nd2", b));
}

// ---------- Parse (dispatcher: commands + UCI + SAN) ----------

TEST_CASE("Parse: quit and exit") {
  Board b;
  auto q = Parse("quit", b);
  auto e = Parse("exit", b);
  CHECK(std::get_if<QuitCmd>(&q));
  CHECK(std::get_if<QuitCmd>(&e));
}

TEST_CASE("Parse: undo") {
  Board b;
  auto u = Parse("undo", b);
  CHECK(std::get_if<UndoCmd>(&u));
}

TEST_CASE("Parse: UCI route returns Move") {
  Board b;
  auto cmd = Parse("e2e4", b);
  auto* m = std::get_if<Move>(&cmd);
  REQUIRE(m);
  CHECK(m->from == Square{1, 4});
  CHECK(m->to == Square{3, 4});
}

TEST_CASE("Parse: SAN route returns Move") {
  Board b;
  auto cmd = Parse("Nf3", b);
  auto* m = std::get_if<Move>(&cmd);
  REQUIRE(m);
  CHECK(m->from == Square{0, 6});
  CHECK(m->to == Square{2, 5});
}

TEST_CASE("Parse: error classification") {
  Board b;
  auto empty = Parse("", b);
  auto bad = Parse("xyz", b);
  auto trailing = Parse("e2e4xx", b);
  auto sq = Parse("z9z9", b);
  auto promo = Parse("e7e8x", b);
  CHECK(std::get<ParseError>(empty) == ParseError::kEmpty);
  CHECK(std::get<ParseError>(bad) == ParseError::kBadSyntax);
  CHECK(std::get<ParseError>(trailing) == ParseError::kBadSyntax);
  CHECK(std::get<ParseError>(sq) == ParseError::kBadSquare);
  CHECK(std::get<ParseError>(promo) == ParseError::kBadPromotion);
}
