#include "board.h"

#include <iostream>

#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"

Board::Board() { Setup(); }

void Board::Setup() {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      grid_[row][col] = nullptr;
    }
  }
  side_to_move_ = Color::kWhite;

  for (int col = 0; col < kSize; ++col) {
    grid_[1][col] = std::make_unique<Pawn>(Color::kWhite);
    grid_[6][col] = std::make_unique<Pawn>(Color::kBlack);
  }
  grid_[0][0] = std::make_unique<Rook>(Color::kWhite);
  grid_[0][7] = std::make_unique<Rook>(Color::kWhite);
  grid_[7][0] = std::make_unique<Rook>(Color::kBlack);
  grid_[7][7] = std::make_unique<Rook>(Color::kBlack);
  grid_[0][1] = std::make_unique<Knight>(Color::kWhite);
  grid_[0][6] = std::make_unique<Knight>(Color::kWhite);
  grid_[7][1] = std::make_unique<Knight>(Color::kBlack);
  grid_[7][6] = std::make_unique<Knight>(Color::kBlack);
  grid_[0][2] = std::make_unique<Bishop>(Color::kWhite);
  grid_[0][5] = std::make_unique<Bishop>(Color::kWhite);
  grid_[7][2] = std::make_unique<Bishop>(Color::kBlack);
  grid_[7][5] = std::make_unique<Bishop>(Color::kBlack);
  grid_[0][3] = std::make_unique<Queen>(Color::kWhite);
  grid_[7][3] = std::make_unique<Queen>(Color::kBlack);
  grid_[0][4] = std::make_unique<King>(Color::kWhite);
  grid_[7][4] = std::make_unique<King>(Color::kBlack);
}

void Board::Print() const {
  for (int row = kSize - 1; row >= 0; --row) {
    std::cout << (row + 1) << ' ';
    for (int col = 0; col < kSize; ++col) {
      if (grid_[row][col]) {
        std::cout << grid_[row][col]->GetSymbol() << ' ';
      } else {
        std::cout << ". ";
      }
    }
    std::cout << '\n';
  }
  std::cout << "  a b c d e f g h\n";
}

Piece* Board::At(const Square& square) const {
  return At(square.row, square.col);
}

Piece* Board::At(int row, int col) const {
  if (!InBounds(row, col)) {
    return nullptr;
  }
  return grid_[row][col].get();
}

bool Board::InBounds(const Square& s) { return InBounds(s.row, s.col); }

bool Board::InBounds(int row, int col) {
  return row >= 0 && row < kSize && col >= 0 && col < kSize;
}

void Board::MovePiece(const Square& from, const Square& to) {
  if (!InBounds(from) || !InBounds(to)) {
    return;
  }
  grid_[to.row][to.col] = std::move(grid_[from.row][from.col]);
  side_to_move_ =
      side_to_move_ == Color::kWhite ? Color::kBlack : Color::kWhite;
}
