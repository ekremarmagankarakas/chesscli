#pragma once

#include <variant>

#include "move.h"
#include "parse_error.h"

struct QuitCmd {};
struct UndoCmd {};

using Command = std::variant<Move, QuitCmd, UndoCmd, ParseError>;
