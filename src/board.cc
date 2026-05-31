#include "board.h"

#include <iostream>

#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"
#include "square.h"

Board::Board() { Setup(); }

Board::Board(const Board& other) : side_to_move_(other.side_to_move_) {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      if (other.grid_[row][col]) {
        grid_[row][col] = other.grid_[row][col]->Clone();
      } else {
        grid_[row][col] = nullptr;
      }
    }
  }
}

Board& Board::operator=(const Board& other) {
  if (this == &other) {
    return *this;
  }
  side_to_move_ = other.side_to_move_;
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      if (other.grid_[row][col]) {
        grid_[row][col] = other.grid_[row][col]->Clone();
      } else {
        grid_[row][col] = nullptr;
      }
    }
  }
  return *this;
}

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

std::optional<Square> Board::FindPiece(const PieceType pt,
                                       const Color color) const {
  Square square;
  bool found = false;
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (piece && piece->GetType() == pt && piece->GetColor() == color) {
        square = {row, col};
        found = true;
        break;
      }
    }
    if (found) {
      return square;
    }
  }
  return std::nullopt;  // Piece Not found
}

bool Board::IsLegal(const Move& move) const {
  if (!InBounds(move.from) || !InBounds(move.to)) {
    return false;
  }

  Piece* piece = At(move.from);
  if (!piece) {
    return false;
  }
  if (!IsYourMove(*piece)) {
    return false;
  }
  if (!IsValidMove(move, *piece)) {
    return false;
  }

  // Promotion required iff pawn reaches last rank.
  int last_rank = piece->GetColor() == Color::kWhite ? 7 : 0;
  bool reaches_last =
      piece->GetType() == PieceType::kPawn && move.to.row == last_rank;
  if (reaches_last != move.promotion.has_value()) {
    return false;
  }

  // Simulate and reject if own king attacked after.
  Board sim = *this;
  sim.Apply(move);
  if (sim.IsInCheck(piece->GetColor())) {
    return false;
  }

  return true;
}

bool Board::IsYourMove(const Piece& piece) const {
  return (piece.GetColor() == side_to_move_);
}

bool Board::IsValidMove(const Move& move, const Piece& piece) const {
  auto moves = piece.ValidMoves(move.from, *this);
  for (const auto& sq : moves) {
    if (sq == move.to) {
      return true;
    }
  }
  return false;
}

bool Board::IsInCheck(Color color) const {
  std::optional<Square> king_square = FindPiece(PieceType::kKing, color);
  if (!king_square) {
    return false;
  }
  return IsSquareAttacked(*king_square, color);
}

bool Board::IsSquareAttacked(Square s, Color color) const {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (piece && piece->GetColor() != color) {
        auto moves = piece->ValidMoves({row, col}, *this);
        if (std::find(moves.begin(), moves.end(), s) != moves.end()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool Board::IsCheckmate() const {
  Color side = side_to_move_;
  if (!IsInCheck(side)) {
    return false;
  }
  return !HasAnyLegalMove(side);
}

bool Board::IsStalemate() const {
  Color side = side_to_move_;
  if (IsInCheck(side)) {
    return false;
  }
  return !HasAnyLegalMove(side);
}

bool Board::HasAnyLegalMove(Color side) const {
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (!piece || piece->GetColor() != side) {
        continue;
      }
      auto moves = piece->ValidMoves({row, col}, *this);
      for (const auto& to : moves) {
        Board sim = *this;
        sim.Apply(Move{Square{row, col}, to, std::nullopt, false, false});
        if (!sim.IsInCheck(side)) {
          return true;
        }
      }
    }
  }
  return false;
}

void Board::Apply(const Move& move) {
  if (move.promotion) {
    Color color = side_to_move_;
    switch (*move.promotion) {
      case PieceType::kQueen:
        grid_[move.to.row][move.to.col] = std::make_unique<Queen>(color);
        break;
      case PieceType::kRook:
        grid_[move.to.row][move.to.col] = std::make_unique<Rook>(color);
        break;
      case PieceType::kBishop:
        grid_[move.to.row][move.to.col] = std::make_unique<Bishop>(color);
        break;
      case PieceType::kKnight:
        grid_[move.to.row][move.to.col] = std::make_unique<Knight>(color);
        break;
      default:
        return;  // Invalid promotion piece
    }
    grid_[move.from.row][move.from.col] = nullptr;
  } else {
    grid_[move.to.row][move.to.col] =
        std::move(grid_[move.from.row][move.from.col]);
  }
  side_to_move_ =
      side_to_move_ == Color::kWhite ? Color::kBlack : Color::kWhite;
}
