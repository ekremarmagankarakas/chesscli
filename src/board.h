#pragma once

#include <array>
#include <memory>
#include <optional>

#include "castling_rights.h"
#include "move.h"
#include "piece.h"
#include "square.h"

class Board {
 public:
  static constexpr int kSize = 8;

  Board();
  Board(const Board& other);
  Board& operator=(const Board& other);
  Board(Board&&) = default;
  Board& operator=(Board&&) = default;

  void Setup();
  void Print() const;

  Piece* At(const Square& square) const;
  Piece* At(int row, int col) const;

  void Apply(const Move& move);

  Color ToMove() const { return side_to_move_; }

  static bool InBounds(const Square& square);
  static bool InBounds(int row, int col);

  bool IsLegal(const Move& move) const;
  bool IsCheckmate() const;
  bool IsStalemate() const;
  bool IsSquareAttacked(Square s, Color color) const;

  bool CanCastleKingside(Color color) const;
  bool CanCastleQueenside(Color color) const;

 private:
  std::array<std::array<std::unique_ptr<Piece>, kSize>, kSize> grid_;
  Color side_to_move_ = Color::kWhite;
  CastlingRights castling_;

  bool IsInCheck(Color color) const;
  bool IsValidMove(const Move& move, const Piece& piece) const;
  bool IsYourMove(const Piece& piece) const;
  bool HasAnyLegalMove(Color side) const;
  std::optional<Square> FindPiece(const PieceType ptype,
                                  const Color color) const;
};
