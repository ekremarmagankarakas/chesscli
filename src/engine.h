#pragma once

#include "move.h"

namespace chess {

class Board;

class Engine {
 public:
  virtual ~Engine() = default;
  virtual Move Choose(Board& board) = 0;
};

}  // namespace chess
