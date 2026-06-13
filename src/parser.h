#pragma once

#include <optional>
#include <string_view>

#include "command.h"
#include "move.h"

namespace chess {

class Board;

// Parse a move in UCI notation: e2e4, e7e8q. Stateless.
std::optional<Move> ParseUCI(std::string_view input);

// Parse a move in SAN notation: e4, Nf3, exd5, O-O, e8=Q, Nbd2, Nxe5+.
// Needs board context to resolve which piece moves.
std::optional<Move> ParseSAN(std::string_view input, const Board& board);

// Dispatch a full input line: commands (quit/undo/...), UCI, or SAN.
Command Parse(std::string_view input, const Board& board);

}  // namespace chess
