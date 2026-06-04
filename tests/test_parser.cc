#include <doctest/doctest.h>

#include <variant>

#include "command.h"
#include "parser.h"

TEST_CASE("UCI e2e4 parses") {
  auto cmd = Parse("e2e4");
  auto* m = std::get_if<Move>(&cmd);
  REQUIRE(m);
  CHECK(m->from == Square{1, 4});
  CHECK(m->to == Square{3, 4});
  CHECK(!m->promotion);
}

TEST_CASE("promotion e7e8q parses") {
  auto cmd = Parse("e7e8q");
  auto* m = std::get_if<Move>(&cmd);
  REQUIRE(m);
  CHECK(m->promotion == PieceType::kQueen);
}

TEST_CASE("quit and exit parse to QuitCmd") {
  auto q = Parse("quit");
  auto e = Parse("exit");
  CHECK(std::get_if<QuitCmd>(&q) != nullptr);
  CHECK(std::get_if<QuitCmd>(&e) != nullptr);
}

TEST_CASE("undo parses to UndoCmd") {
  auto u = Parse("undo");
  CHECK(std::get_if<UndoCmd>(&u) != nullptr);
}

TEST_CASE("garbage produces ParseError") {
  auto empty = Parse("");
  auto bad = Parse("xyz");
  auto trailing = Parse("e2e4xx");
  auto sq = Parse("z9z9");
  auto promo = Parse("e7e8x");
  REQUIRE(std::get_if<ParseError>(&empty));
  REQUIRE(std::get_if<ParseError>(&bad));
  REQUIRE(std::get_if<ParseError>(&trailing));
  REQUIRE(std::get_if<ParseError>(&sq));
  REQUIRE(std::get_if<ParseError>(&promo));
  CHECK(*std::get_if<ParseError>(&empty) == ParseError::kEmpty);
  CHECK(*std::get_if<ParseError>(&bad) == ParseError::kBadSyntax);
  CHECK(*std::get_if<ParseError>(&trailing) == ParseError::kBadSyntax);
  CHECK(*std::get_if<ParseError>(&sq) == ParseError::kBadSquare);
  CHECK(*std::get_if<ParseError>(&promo) == ParseError::kBadPromotion);
}
