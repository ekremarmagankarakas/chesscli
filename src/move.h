#pragma once

#include <optional>
#include <string>

#include "piece.h"
#include "square.h"

namespace chess {

struct Move {
  Square from;
  Square to;
  std::optional<PieceType> promotion;
};

inline std::string MoveToUCI(const Move& m) {
  std::string s = m.from.ToAlgebraic() + m.to.ToAlgebraic();
  if (m.promotion) {
    switch (*m.promotion) {
      case PieceType::kQueen:
        s += 'q';
        break;
      case PieceType::kRook:
        s += 'r';
        break;
      case PieceType::kBishop:
        s += 'b';
        break;
      case PieceType::kKnight:
        s += 'n';
        break;
      default:
        s += '?';
        break;
    }
  }
  return s;
}

}  // namespace chess
