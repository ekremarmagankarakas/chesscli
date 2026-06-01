#pragma once

#include <optional>

#include "piece.h"
#include "square.h"

struct Move {
  Square from;
  Square to;
  std::optional<PieceType> promotion;
};
