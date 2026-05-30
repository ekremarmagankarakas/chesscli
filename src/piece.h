#pragma once
#include <vector>

enum class Color { kWhite, kBlack };
enum class PieceType { kPawn, kKnight, kBishop, kRook, kQueen, kKing };

struct Square {
  int row;  // rank 0 to 7
  int col;  // file 0 to 7 (a to h)
};

class Board;  // Forward declaration

class Piece {
 public:
  Piece(Color color, PieceType type);
  virtual ~Piece();

  Color GetColor() const;
  PieceType GetType() const;

  virtual char GetSymbol() const = 0;
  virtual std::vector<Square> ValidMoves(const Square& from,
                                         const Board& board) const = 0;

 private:
  Color color_;
  PieceType type_;
};
