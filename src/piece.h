#pragma once
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

 private:
  Color color_;
  PieceType type_;
};
