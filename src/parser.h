#pragma once

#include <optional>
#include <string>

#include "move.h"

class Parser {
 public:
  Parser(const std::string& input);

  std::optional<Move> parse();

 private:
  std::string input_;
};
