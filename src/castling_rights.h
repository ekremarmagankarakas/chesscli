#pragma once

#include "piece.h"
#include "square.h"

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

  bool Kingside(Color c) const { return c == Color::kWhite ? wk : bk; }
  bool Queenside(Color c) const { return c == Color::kWhite ? wq : bq; }
};
