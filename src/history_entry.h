#pragma once

#include <memory>

#include "move.h"

class Board;  // forward declaration

struct HistoryEntry {
  std::unique_ptr<const Board> pre_state;
  Move move;
};
