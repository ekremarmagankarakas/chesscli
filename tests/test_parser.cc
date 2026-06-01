#include <doctest/doctest.h>

#include "parser.h"

TEST_CASE("UCI e2e4 parses") {
  auto m = Parser("e2e4").parse();
  REQUIRE(m);
  CHECK(m->from == Square{1, 4});
  CHECK(m->to == Square{3, 4});
  CHECK(!m->promotion);
}

TEST_CASE("promotion e7e8q parses") {
  auto m = Parser("e7e8q").parse();
  REQUIRE(m);
  CHECK(m->promotion == PieceType::kQueen);
}

TEST_CASE("garbage rejected") {
  CHECK_FALSE(Parser("").parse());
  CHECK_FALSE(Parser("xyz").parse());
  CHECK_FALSE(Parser("e2e4xx").parse());
}
