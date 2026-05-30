#include "board.h"

#include "pieces/bishop.h"
#include "pieces/king.h"
// #include "pawn.h"
// #include "rook.h"
// #include "knight.h"
// #include "queen.h"

#include <iostream>

Board::Board() { Setup(); }

void Board::Setup() {
  // Clear the board.
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      grid_[row][col] = nullptr;
    }
  }
  // Place Starting Pawns
  // for (int col = 0; col < kSize; ++col) {
  //   grid_[1][col] = std::make_unique<Pawn>(Color::kWhite, PieceType::kPawn);
  //   grid_[6][col] = std::make_unique<Pawn>(Color::kBlack, PieceType::kPawn);
  // }
  // Rooks
  // grid_[0][0] = std::make_unique<Rook>(Color::kWhite, PieceType::kRook);
  // grid_[0][7] = std::make_unique<Rook>(Color::kWhite, PieceType::kRook);
  // grid_[7][0] = std::make_unique<Rook>(Color::kBlack, PieceType::kRook);
  // grid_[7][7] = std::make_unique<Rook>(Color::kBlack, PieceType::kRook);
  // // Knights
  // grid_[0][1] = std::make_unique<Knight>(Color::kWhite, PieceType::kKnight);
  // grid_[0][6] = std::make_unique<Knight>(Color::kWhite, PieceType::kKnight);
  // grid_[7][1] = std::make_unique<Knight>(Color::kBlack, PieceType::kKnight);
  // grid_[7][6] = std::make_unique<Knight>(Color::kBlack, PieceType::kKnight);
  // Bishops
  grid_[0][2] = std::make_unique<Bishop>(Color::kWhite, PieceType::kBishop);
  grid_[0][5] = std::make_unique<Bishop>(Color::kWhite, PieceType::kBishop);
  grid_[7][2] = std::make_unique<Bishop>(Color::kBlack, PieceType::kBishop);
  grid_[7][5] = std::make_unique<Bishop>(Color::kBlack, PieceType::kBishop);
  // Queens
  // grid_[0][3] = std::make_unique<Queen>(Color::kWhite, PieceType::kQueen);
  // grid_[7][3] = std::make_unique<Queen>(Color::kBlack, PieceType::kQueen);
  // Kings
  grid_[0][4] = std::make_unique<King>(Color::kWhite, PieceType::kKing);
  grid_[7][4] = std::make_unique<King>(Color::kBlack, PieceType::kKing);
}

void Board::Print() const {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      if (grid_[row][col]) {
        std::cout << grid_[row][col]->GetSymbol() << ' ';
      } else {
        std::cout << ". ";
      }
    }
    std::cout << std::endl;
  }
}

Piece* Board::At(const Square& square) const {
  if (InBounds(square)) {
    return grid_[square.row][square.col].get();
  }
  return nullptr;
}
Piece* Board::At(int row, int col) const {
  if (InBounds({row, col})) {
    return grid_[row][col].get();
  }
  return nullptr;
}

bool Board::InBounds(const Square& s) {
  return s.row >= 0 && s.row < kSize && s.col >= 0 && s.col < kSize;
}

bool Board::InBounds(int row, int col) {
  return row >= 0 && row < kSize && col >= 0 && col < kSize;
}

void Board::MovePiece(const Square& from, const Square& to) {
  if (InBounds(from) && InBounds(to)) {
    grid_[to.row][to.col] = std::move(grid_[from.row][from.col]);
    grid_[from.row][from.col] = nullptr;
  }
}
