#include "parser.h"

#include <optional>
#include <string_view>

#include "move.h"
#include "piece.h"
#include "square.h"

Command Parse(std::string_view input) {
  if (input.empty()) {
    return ParseError::kEmpty;
  }
  if (input == "quit" || input == "exit") {
    return QuitCmd{};
  }
  if (input == "undo") {
    return UndoCmd{};
  }
  if (input == "resign") {
    return ResignCmd{};
  }
  if (input.size() < 4 || input.size() > 5) {
    return ParseError::kBadSyntax;
  }

  auto from = Square::FromAlgebraic(input.substr(0, 2));
  auto to = Square::FromAlgebraic(input.substr(2, 2));
  if (!from || !to) {
    return ParseError::kBadSquare;
  }

  std::optional<PieceType> promotion;
  if (input.size() == 5) {
    switch (input[4]) {
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
        return ParseError::kBadPromotion;
    }
  }

  return Move{*from, *to, promotion};
}
