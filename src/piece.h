#pragma once
#include <memory>
#include <vector>

#include "square.h"

enum class Color { kWhite, kBlack };
enum class PieceType { kPawn, kKnight, kBishop, kRook, kQueen, kKing };

class Board;  // Forward declaration

class Piece {
 public:
  Piece(Color color, PieceType type);
  virtual ~Piece();

  virtual std::unique_ptr<Piece> Clone() const = 0;
  Color GetColor() const;
  PieceType GetType() const;

  virtual char GetSymbol() const = 0;
  virtual std::vector<Square> ValidMoves(const Square& from,
                                         const Board& board) const = 0;

  // Squares this piece attacks (controls). Default: same as ValidMoves.
  // Pawn overrides to exclude pushes; King overrides to exclude castling.
  // Used by IsSquareAttacked. Must not invoke any logic that recurses
  // back into IsSquareAttacked.
  virtual std::vector<Square> Attacks(const Square& from,
                                      const Board& board) const {
    return ValidMoves(from, board);
  }

 private:
  Color color_;
  PieceType type_;
};
