#include "board.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>

#include "pieces/bishop.h"
#include "pieces/king.h"
#include "pieces/knight.h"
#include "pieces/pawn.h"
#include "pieces/queen.h"
#include "pieces/rook.h"
#include "square.h"

Board::Board() { Setup(); }

Board::Board(const Board& other)
    : side_to_move_(other.side_to_move_), castling_(other.castling_) {
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
  castling_ = other.castling_;
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
  castling_ = CastlingRights{};

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

std::optional<Square> Board::FindPiece(const PieceType ptype,
                                       const Color color) const {
  Square square;
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      auto piece = At(row, col);
      if (piece && piece->GetType() == ptype && piece->GetColor() == color) {
        square = {row, col};
        return square;
      }
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
  sim.ApplyNoHistory(move);
  if (sim.IsInCheck(piece->GetColor())) {
    return false;
  }

  return true;
}

std::optional<Move> Board::LastMove() const {
  if (history_.empty()) {
    return std::nullopt;
  }
  return history_.back().move;
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
        auto attacks = piece->Attacks({row, col}, *this);
        if (std::find(attacks.begin(), attacks.end(), s) != attacks.end()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool Board::CanCastleKingside(Color color) const {
  if (!castling_.Kingside(color)) {
    return false;
  }
  int row = color == Color::kWhite ? 0 : 7;
  if (At(row, 5) || At(row, 6)) {
    return false;
  }
  if (IsSquareAttacked({row, 4}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 5}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 6}, color)) {
    return false;
  }
  return true;
}

bool Board::CanCastleQueenside(Color color) const {
  if (!castling_.Queenside(color)) {
    return false;
  }
  int row = color == Color::kWhite ? 0 : 7;
  if (At(row, 1) || At(row, 2) || At(row, 3)) {
    return false;
  }
  if (IsSquareAttacked({row, 4}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 3}, color)) {
    return false;
  }
  if (IsSquareAttacked({row, 2}, color)) {
    return false;
  }
  return true;
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
        sim.ApplyNoHistory(Move{Square{row, col}, to, std::nullopt});
        if (!sim.IsInCheck(side)) {
          return true;
        }
      }
    }
  }
  return false;
}

void Board::Undo() {
  if (history_.empty()) {
    return;
  }
  const Board& snap = *history_.back().pre_state;
  for (int row = 0; row < kSize; ++row) {
    for (int col = 0; col < kSize; ++col) {
      if (snap.grid_[row][col]) {
        grid_[row][col] = snap.grid_[row][col]->Clone();
      } else {
        grid_[row][col] = nullptr;
      }
    }
  }
  side_to_move_ = snap.side_to_move_;
  castling_ = snap.castling_;
  history_.pop_back();
}

void Board::Apply(const Move& move) {
  history_.push_back(HistoryEntry{std::make_unique<const Board>(*this), move});
  ApplyNoHistory(move);
}

void Board::ApplyNoHistory(const Move& move) {
  Piece* moving = At(move.from);
  if (!moving) {
    return;
  }
  Color color = moving->GetColor();
  PieceType ptype = moving->GetType();

  // En passant: pawn moves diagonally to empty square -> remove the
  // adjacent pawn that just double-pushed.
  bool is_ep_capture =
      ptype == PieceType::kPawn && move.from.col != move.to.col && !At(move.to);
  if (is_ep_capture) {
    grid_[move.from.row][move.to.col] = nullptr;
  }

  // Castling: king moves 2 columns.
  bool is_castle =
      ptype == PieceType::kKing && std::abs(move.to.col - move.from.col) == 2;
  if (is_castle) {
    int rook_from = move.to.col > move.from.col ? 7 : 0;
    int rook_to = move.to.col > move.from.col ? 5 : 3;
    grid_[move.from.row][rook_to] = std::move(grid_[move.from.row][rook_from]);
  }

  if (move.promotion) {
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
        return;
    }
    grid_[move.from.row][move.from.col] = nullptr;
  } else {
    grid_[move.to.row][move.to.col] =
        std::move(grid_[move.from.row][move.from.col]);
  }

  // Update castling rights.
  if (ptype == PieceType::kKing) {
    castling_.ClearAll(color);
  }
  if (ptype == PieceType::kRook) {
    castling_.ClearByRookSquare(move.from);
  }
  castling_.ClearByRookSquare(move.to);  // captured rook on home square

  side_to_move_ =
      side_to_move_ == Color::kWhite ? Color::kBlack : Color::kWhite;
}
