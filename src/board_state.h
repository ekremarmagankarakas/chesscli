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

// Position equality for threefold repetition.
// halfmove_clock intentionally excluded: same position with different clock
// still counts as repetition.
inline bool operator==(const BoardState& a, const BoardState& b) {
  return a.squares == b.squares && a.side == b.side &&
         a.castling == b.castling && a.ep_file == b.ep_file;
}
