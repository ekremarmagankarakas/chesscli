#include "parser.h"

#include <optional>

#include "move.h"
#include "piece.h"

Parser::Parser(const std::string& input) : input_(input) {}

std::optional<Move> Parser::parse() {
  if (input_.size() < 4) {
    return std::nullopt;
  }
  auto from = Square::FromAlgebraic(input_.substr(0, 2));
  auto to = Square::FromAlgebraic(input_.substr(2, 2));
  if (!from || !to) {
    return std::nullopt;
  }
  return Move(*from, *to, std::nullopt, false, false);
}
