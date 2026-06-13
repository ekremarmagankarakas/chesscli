#pragma once

#include "piece.h"
#include "square.h"

namespace chess {

struct CastlingRights {
  bool wk = true, wq = true, bk = true, bq = true;

  void ClearAll(Color c) {
    if (c == Color::kWhite) {
      wk = wq = false;
    } else {
      bk = bq = false;
    }
  }

  void ClearByRookSquare(const Square& sq) {
    if (sq == Square{0, 0}) {
      wq = false;
    }
    if (sq == Square{0, 7}) {
      wk = false;
    }
    if (sq == Square{7, 0}) {
      bq = false;
    }
    if (sq == Square{7, 7}) {
      bk = false;
    }
  }

  uint8_t Snapshot() const {
    // Bit 0: white kingside castling right
    // Bit 1: white queenside castling right
    // Bit 2: black kingside castling right
    // Bit 3: black queenside castling right
    uint8_t result = 0;
    if (wk) {
      result |= 1;
    }
    if (wq) {
      result |= 2;
    }
    if (bk) {
      result |= 4;
    }
    if (bq) {
      result |= 8;
    }
    return result;
  }

  void Restore(uint8_t code) {
    wk = (code & 1) != 0;
    wq = (code & 2) != 0;
    bk = (code & 4) != 0;
    bq = (code & 8) != 0;
  }

  bool Kingside(Color c) const { return c == Color::kWhite ? wk : bk; }
  bool Queenside(Color c) const { return c == Color::kWhite ? wq : bq; }
};

}  // namespace chess
