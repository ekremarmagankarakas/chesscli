#include <doctest/doctest.h>

#include <cstdint>
#include <optional>
#include <vector>

#include "board.h"

static uint64_t Perft(Board& b, int depth) {
  if (depth == 0) {
    return 1;
  }
  uint64_t nodes = 0;
  for (int r = 0; r < 8; ++r) {
    for (int c = 0; c < 8; ++c) {
      Piece* p = b.At(r, c);
      if (!p || p->GetColor() != b.ToMove()) {
        continue;
      }
      for (Square to : p->ValidMoves({r, c}, b)) {
        bool is_promo = p->GetType() == PieceType::kPawn &&
                        (to.row == 0 || to.row == 7);
        std::vector<std::optional<PieceType>> promos =
            is_promo ? std::vector<std::optional<PieceType>>{
                           PieceType::kQueen, PieceType::kRook,
                           PieceType::kBishop, PieceType::kKnight}
                     : std::vector<std::optional<PieceType>>{std::nullopt};
        for (auto promo : promos) {
          Move m{Square{r, c}, to, promo};
          if (!b.IsLegal(m)) {
            continue;
          }
          Board sim = b;
          sim.Apply(m);
          nodes += Perft(sim, depth - 1);
        }
      }
    }
  }
  return nodes;
}

TEST_CASE("perft starting position depth 1") {
  Board b;
  CHECK(Perft(b, 1) == 20);
}

TEST_CASE("perft starting position depth 2") {
  Board b;
  CHECK(Perft(b, 2) == 400);
}
