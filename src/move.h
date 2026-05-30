#pragma once

#include <optional>

#include "piece.h"

struct Move {
  Square from;
  Square to;
  std::optional<PieceType> promotion;
  bool is_castle = false;
  bool is_en_passant = false;
};
