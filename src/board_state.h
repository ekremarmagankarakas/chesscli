#pragma once

#include <array>
#include <cstdint>

struct BoardState {
  std::array<uint8_t, 64> squares;
  uint8_t side;
  uint8_t castling;
  int8_t ep_file;
  int halfmove_clock;
};
