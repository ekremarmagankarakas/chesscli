#pragma once

namespace chess {

enum class ParseError {
  kEmpty,
  kBadSyntax,
  kBadSquare,
  kBadPromotion,
};

}  // namespace chess
