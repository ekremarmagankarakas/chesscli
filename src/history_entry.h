#pragma once

#include "board_state.h"
#include "move.h"

namespace chess {

struct HistoryEntry {
  BoardState pre_state;
  Move move;
};

}  // namespace chess
