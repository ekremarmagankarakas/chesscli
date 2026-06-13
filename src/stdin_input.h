#pragma once

#include "input_source.h"

namespace chess {

class StdinInput : public InputSource {
 public:
  std::optional<std::string> ReadLine() override;
};

}  // namespace chess
