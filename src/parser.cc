#include "parser.h"

#include <optional>

#include "move.h"
#include "piece.h"
#include "square.h"

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

  std::optional<PieceType> promotion;
  if (input_.size() == 5) {
    switch (input_[4]) {
      case 'q':
        promotion = PieceType::kQueen;
        break;
      case 'r':
        promotion = PieceType::kRook;
        break;
      case 'b':
        promotion = PieceType::kBishop;
        break;
      case 'n':
        promotion = PieceType::kKnight;
        break;
      default:
        return std::nullopt;
    }
  }

  return Move(*from, *to, promotion);
}
