#include "piece.h"

Piece::Piece(Color color, PieceType type) : color_(color), type_(type) {}

Piece::~Piece() = default;

Color Piece::GetColor() const { return color_; }

PieceType Piece::GetType() const { return type_; }
